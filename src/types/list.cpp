// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "build_options.h"
#include "debug_valid_objects.h"
#include "importing_macros.h"
#include "tagged_value.h"
#include "testing.h"

#include "type.h"

#include "list.h"

namespace circa {
namespace list_t {

    struct ListData {
        int refCount;
        int count;
        int capacity;
        TaggedValue items[0];
        // items has size [capacity].
    };

    ListData* touch(ListData* original);

    void assert_valid_list(ListData* list)
    {
        if (list == NULL) return;
        debug_assert_valid_object(list, LIST_OBJECT);
        if (list->refCount == 0) {
            std::stringstream err;
            err << "list has zero refs: " << list;
            internal_error(err.str().c_str());
        }
        ca_assert(list->refCount > 0);
    }

    void incref(ListData* data)
    {
        assert_valid_list(data);
        data->refCount++;

        //std::cout << "incref " << data << " to " << data->refCount << std::endl;
    }

    void decref(ListData* data)
    {
        assert_valid_list(data);
        ca_assert(data->refCount > 0);
        data->refCount--;

        if (data->refCount == 0) {
            // Release all elements
            for (int i=0; i < data->count; i++)
                set_null(&data->items[i]);
            free(data);
            debug_unregister_valid_object(data);
        }

        //std::cout << "decref " << data << " to " << data->refCount << std::endl;
    }

    ListData* create_list(int capacity)
    {
        ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
        debug_register_valid_object(result, LIST_OBJECT);

        result->refCount = 1;
        result->count = 0;
        result->capacity = capacity;
        memset(result->items, 0, capacity * sizeof(TaggedValue));
        for (int i=0; i < capacity; i++)
            result->items[i].init();

        //std::cout << "created list " << result << std::endl;

        return result;
    }

    ListData* duplicate(ListData* source)
    {
        if (source == NULL || source->count == 0)
            return NULL;

        assert_valid_list(source);

        ListData* result = create_list(source->capacity);

        result->count = source->count;

        for (int i=0; i < source->count; i++)
            copy(&source->items[i], &result->items[i]);

        return result;
    }

    ListData* increase_capacity(ListData* original, int new_capacity)
    {
        if (original == NULL)
            return create_list(new_capacity);

        assert_valid_list(original);
        ListData* result = create_list(new_capacity);

        bool createCopy = original->refCount > 1;

        result->count = original->count;
        for (int i=0; i < result->count; i++) {
            TaggedValue* left = &original->items[i];
            TaggedValue* right = &result->items[i];
            if (createCopy)
                copy(left, right);
            else
                swap(left, right);
        }

        decref(original);
        return result;
    }

    ListData* double_capacity(ListData* original)
    {
        if (original == NULL)
            return create_list(1);

        ListData* result = increase_capacity(original, original->capacity * 2);
        return result;
    }

    ListData* resize(ListData* original, int numElements)
    {
        if (original == NULL) {
            if (numElements == 0)
                return NULL;
            ListData* result = create_list(numElements);
            result->count = numElements;
            return result;
        }

        if (numElements == 0) {
            decref(original);
            return NULL;
        }

        // Check for not enough capacity
        if (numElements > original->capacity) {
            ListData* result = increase_capacity(original, numElements);
            result->count = numElements;
            return result;
        }

        if (original->count == numElements)
            return original;

        // Capacity is good, will need to modify 'count' on list and possibly
        // set some items to null. This counts as a modification.
        ListData* result = touch(original);

        // Possibly set extra elements to null, if we are shrinking.
        for (int i=numElements; i < result->count; i++)
            set_null(&result->items[i]);
        result->count = numElements;

        return result;
    }

    void clear(ListData** data)
    {
        if (*data == NULL) return;
        decref(*data);
        *data = NULL;
    }

    ListData* touch(ListData* original)
    {
        if (original == NULL)
            return NULL;
        ca_assert(original->refCount > 0);
        if (original->refCount == 1)
            return original;

        ListData* copy = duplicate(original);
        decref(original);
        return copy;
    }

    TaggedValue* append(ListData** data)
    {
        if (*data == NULL) {
            *data = create_list(1);
        } else {
            *data = touch(*data);
            
            if ((*data)->count == (*data)->capacity)
                *data = double_capacity(*data);
        }

        ListData* d = *data;
        d->count++;
        return &d->items[d->count - 1];
    }

    TaggedValue* get_index(ListData* list, int index)
    {
        if (list == NULL)
            return NULL;
        if (index >= list->count)
            return NULL;
        return &list->items[index];
    }

    void set_index(ListData** data, int index, TaggedValue* v)
    {
        *data = touch(*data);
        copy(v, get_index(*data, index));
    }

    int num_elements(ListData* list)
    {
        if (list == NULL) return 0;
        return list->count;
    }

    int refcount(ListData* value)
    {
        if (value == NULL) return 0;
        return value->refCount;
    }

    void remove_and_replace_with_back(ListData** data, int index)
    {
        *data = touch(*data);
        ca_assert(index < (*data)->count);

        set_null(&(*data)->items[index]);

        int lastElement = (*data)->count - 1;
        if (index < lastElement)
            swap(&(*data)->items[index], &(*data)->items[lastElement]);

        (*data)->count--;
    }

    void remove_nulls(ListData** dataPtr)
    {
        if (*dataPtr == NULL)
            return;

        *dataPtr = touch(*dataPtr);
        ListData* data = *dataPtr;

        int numRemoved = 0;
        for (int i=0; i < data->count; i++) {
            if (is_null(&data->items[i]))
                numRemoved++;
            else
                swap(&data->items[i - numRemoved], &data->items[i]);
        }
        *dataPtr = resize(*dataPtr, data->count - numRemoved);
    }

    TaggedValue* prepend(ListData** data)
    {
        append(data);

        ListData* d = *data;

        for (int i=d->count - 1; i >= 1; i--)
            swap(&d->items[i], &d->items[i - 1]);

        return &d->items[0];
    }

    std::string to_string(ListData* value)
    {
        if (value == NULL)
            return "[]";

        std::stringstream out;
        out << "[";
        for (int i=0; i < value->count; i++) {
            if (i > 0) out << ", ";
            out << to_string(&value->items[i]);
        }
        out << "]";
        return out.str();
    }

    // TaggedValue wrappers
    void tv_touch(TaggedValue* value);

    void resize(TaggedValue* list, int newSize)
    {
        ca_assert(is_list(list));
        set_pointer(list, resize((ListData*) get_pointer(list), newSize));
    }

    void clear(TaggedValue* list)
    {
        ca_assert(is_list(list));
        clear((ListData**) &list->value_data);
    }

    TaggedValue* append(TaggedValue* list)
    {
        ca_assert(is_list(list));
        return append((ListData**) &list->value_data);
    }
    TaggedValue* prepend(TaggedValue* list)
    {
        ca_assert(is_list(list));
        return prepend((ListData**) &list->value_data);
    }

    void remove_nulls(TaggedValue* list)
    {
        ca_assert(is_list(list));
        remove_nulls((ListData**) &list->value_data);
    }

    void tv_initialize(Type* type, TaggedValue* value)
    {
        ca_assert(value->value_data.ptr == NULL);

        // If type has a prototype then initialize to that.
        Branch& prototype = type_t::get_prototype(type);
        if (prototype.length() > 0) {
            List* list = List::checkCast(value);
            list->resize(prototype.length());

            for (int i=0; i < prototype.length(); i++)
                change_type(list->get(i), type_contents(prototype[i]->type));
        }
    }

    void tv_release(TaggedValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        if (data == NULL) return;
        decref(data);
    }

    void tv_copy(TaggedValue* source, TaggedValue* dest)
    {
        ca_assert(is_list(source));
        ca_assert(is_list(dest));
        ListData* s = (ListData*) get_pointer(source);
        ListData* d = (ListData*) get_pointer(dest);

    #if DISABLE_LIST_VALUE_SHARING
        if (d != NULL) decref(d);
        set_pointer(dest, duplicate(s));
    #else
        if (s != NULL) incref(s);
        if (d != NULL) decref(d);
        set_pointer(dest, s);
    #endif
    }

    bool tv_equals(TaggedValue* leftValue, TaggedValue* right)
    {
        ca_assert(is_list(leftValue));
        Type* rhsType = right->value_type;
        if (rhsType->numElements == NULL || rhsType->getIndex == NULL)
            return false;

        List* left = List::checkCast(leftValue);

        int leftCount = left->numElements();

        if (leftCount != right->numElements())
            return false;

        for (int i=0; i < leftCount; i++) {
            if (!circa::equals(left->get(i), right->getIndex(i)))
                return false;
        }
        return true;
    }

    void tv_cast(CastResult* result, TaggedValue* source, Type* type,
        TaggedValue* dest, bool checkOnly)
    {
        List* sourceList = List::checkCast(source);

        if (sourceList == NULL) {
            result->success = false;
            return;
        }

        int sourceLength = sourceList->length();
        int prototypeLength = type->prototype.length();

        // if prototypeLength is 0 then this is a freeform list, so just do a
        // copy and call it a day.
        if (prototypeLength == 0) {
            if (!checkOnly) {
                copy(source, dest);
                dest->value_type = type;
            }
            return;
        }

        // check for correct # of elements
        if (sourceLength != prototypeLength) {
            result->success = false;
            return;
        }

        List* destList = NULL;

        if (!checkOnly) {
            destList = set_list(dest);
            destList->resize(sourceLength);
            dest->value_type = type;
        }

        for (int i=0; i < sourceLength; i++) {
            TaggedValue* sourceElement = sourceList->get(i);
            Type* elementType = get_compound_list_element_type(type, i);

            TaggedValue* destElement = NULL;
            if (!checkOnly)
                destElement = destList->get(i);
        
            cast(result, sourceElement, elementType, destElement, checkOnly);

            if (!result->success)
                return;
        }

        if (!checkOnly)
            dest->value_type = type;
    }

    TaggedValue* tv_get_index(TaggedValue* value, int index)
    {
        ca_assert(is_list(value));
        ListData* s = (ListData*) get_pointer(value);
        return get_index(s, index);
    }

    void tv_set_index(TaggedValue* value, int index, TaggedValue* element)
    {
        ca_assert(is_list(value));
        ListData* s = (ListData*) get_pointer(value);
        set_index(&s, index, element);
        set_pointer(value, s);
    }

    TaggedValue* tv_get_field(TaggedValue* value, const char* fieldName)
    {
        int index = value->value_type->findFieldIndex(fieldName);
        if (index < 0)
            return NULL;
        return tv_get_index(value, index);
    }

    int tv_num_elements(TaggedValue* value)
    {
        ca_assert(is_list(value));
        ListData* s = (ListData*) get_pointer(value);
        return num_elements(s);
    }

    std::string tv_to_string(TaggedValue* value)
    {
        ca_assert(is_list(value));
        return to_string((ListData*) get_pointer(value));
    }

    void tv_touch(TaggedValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        set_pointer(value, touch(data));
    }

    void tv_static_type_query(Type* type, StaticTypeQuery* result)
    {
        Term* term = result->targetTerm;
        Branch& prototype = type->prototype;
        
        // If prototype is empty then accept any list
        if (prototype.length() == 0) {
            if (is_list_based_type(type_contents(term->type)))
                return result->succeed();
            else
                return result->fail();
        }

        // Inspect a call to list(), look at inputs instead of looking at the result.
        if (term->function == LIST_FUNC)
        {
            if (term->numInputs() != prototype.length())
                return result->fail();

            for (int i=0; i < prototype.length(); i++)
                if (!circa::term_output_always_satisfies_type(
                            term->input(i), type_contents(prototype[i]->type)))
                    return result->fail();

            return result->succeed();
        }

        if (is_subtype(type, type_contents(term->type)))
            return result->succeed();
        else
            return result->fail();
    }

    bool tv_is_subtype(Type* type, Type* otherType)
    {
        if (!is_list_based_type(otherType))
            return false;

        // Check if our type has a prototype. If there's no prototype
        // then any list can be a subtype.
        Branch& prototype = type->prototype;

        if (prototype.length() == 0)
            return true;

        Branch& otherPrototype = otherType->prototype;
        if (prototype.length() != otherType->prototype.length())
            return false;

        // Check each element
        for (int i=0; i < prototype.length(); i++)
            if (!circa::is_subtype(type_contents(prototype[i]->type),
                        type_contents(otherPrototype[i]->type)))
                return false;

        return true;
    }

    bool tv_value_fits_type(Type* type, TaggedValue* value)
    {
        if (!is_list(value))
            return false;

        Branch& prototype = type->prototype;
        if (prototype.length() == 0)
            return true;

        int numElements = value->numElements();
        if (prototype.length() != numElements)
            return false;

        for (int i=0; i < numElements; i++)
            if (!circa::value_fits_type(value->getIndex(i),
                        type_contents(prototype[i]->type)))
                return false;
        return true;
    }

    void remove_and_replace_with_back(TaggedValue* value, int index)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        remove_and_replace_with_back(&data, index);
        set_pointer(value, data);
    }

    bool is_list_based_type(Type* type)
    {
        return type->initialize == tv_initialize;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->initialize = tv_initialize;
        type->release = tv_release;
        type->copy = tv_copy;
        type->toString = tv_to_string;
        type->equals = tv_equals;
        type->cast = tv_cast;
        type->getIndex = tv_get_index;
        type->setIndex = tv_set_index;
        type->getField = tv_get_field;
        type->numElements = tv_num_elements;
        type->touch = tv_touch;
        type->staticTypeQuery = tv_static_type_query;
        type->isSubtype = tv_is_subtype;
        type->valueFitsType = tv_value_fits_type;
    }

    CA_FUNCTION(append)
    {
        set_list(OUTPUT);
        List* result = List::checkCast(OUTPUT);
        copy(INPUT(0), OUTPUT);
        TaggedValue* value = INPUT(1);
        copy(value, result->append());
    }

    CA_FUNCTION(count)
    {
        List* list = List::checkCast(INPUT(0));
        set_int(OUTPUT, list->length());
    }

    void postponed_setup_type(Term* type)
    {
        Term* list_append =
            import_member_function(type, append, "append(List, any) -> List");
        function_set_use_input_as_output(list_append, 0, true);
        import_member_function(type, count, "count(List) -> int");
    }

    namespace tests {

        void test_simple()
        {
            List list;
            test_assert(list.length() == 0);
            list.append();
            test_assert(list.length() == 1);
            list.append();
            test_assert(list.length() == 2);
            list.clear();
            test_assert(list.length() == 0);
        }

        void test_tagged_value()
        {
            TypeRef list = Type::create();
            list_t::setup_type(list);

            TaggedValue value;
            change_type(&value, list);

            test_equals(to_string(&value), "[]");
            test_assert(get_index(&value, 1) == NULL);
            test_assert(num_elements(&value) == 0);

            set_int(list_t::append(&value), 1);
            set_int(list_t::append(&value), 2);
            set_int(list_t::append(&value), 3);

            test_equals(to_string(&value), "[1, 2, 3]");

            test_assert(as_int(get_index(&value, 1)) == 2);
            test_assert(num_elements(&value) == 3);
        }

        void test_tagged_value_copy()
        {
            TypeRef list = Type::create();
            list_t::setup_type(list);

            TaggedValue value(list);

            set_int(list_t::append(&value), 1);
            set_int(list_t::append(&value), 2);
            set_int(list_t::append(&value), 3);

            test_equals(to_string(&value), "[1, 2, 3]");

            TaggedValue value2;
            test_assert(value.value_type->copy != NULL);
            copy(&value, &value2);

            test_equals(to_string(&value2), "[1, 2, 3]");

            set_int(list_t::append(&value2), 4);

            test_equals(to_string(&value), "[1, 2, 3]");
            test_equals(to_string(&value2), "[1, 2, 3, 4]");
        }

        void test_touch()
        {
            TypeRef list = Type::create();
            list_t::setup_type(list);

            TaggedValue value(list);

            set_int(list_t::append(&value), 1);
            set_int(list_t::append(&value), 2);

            TaggedValue value2(list);
            copy(&value, &value2);

            #if !DISABLE_LIST_VALUE_SHARING
            test_assert(get_pointer(&value) == get_pointer(&value2));
            #endif
            touch(&value2);
            test_assert(get_pointer(&value) != get_pointer(&value2));
        }

        void test_prepend()
        {
            TypeRef list = Type::create();
            list_t::setup_type(list);

            TaggedValue value(list);

            set_int(list_t::append(&value), 1);
            set_int(list_t::append(&value), 2);

            test_assert(to_string(&value) == "[1, 2]");
            list_t::prepend(&value);
            test_assert(to_string(&value) == "[null, 1, 2]");
            set_int(list_t::tv_get_index(&value, 0), 4);
            test_assert(to_string(&value) == "[4, 1, 2]");

            reset(&value);

            test_assert(to_string(&value) == "[]");
            list_t::prepend(&value);
            test_assert(to_string(&value) == "[null]");

            reset(&value);

            list_t::prepend(&value);
            set_int(list_t::tv_get_index(&value, 0), 1);
            test_assert(to_string(&value) == "[1]");
            list_t::prepend(&value);
            test_assert(to_string(&value) == "[null, 1]");
        }

        void register_tests()
        {
            REGISTER_TEST_CASE(list_t::tests::test_simple);
            REGISTER_TEST_CASE(list_t::tests::test_tagged_value);
            REGISTER_TEST_CASE(list_t::tests::test_tagged_value_copy);
            REGISTER_TEST_CASE(list_t::tests::test_touch);
            REGISTER_TEST_CASE(list_t::tests::test_prepend);
        }

    } // namespace tests
} // namespace list_t

List::List()
  : TaggedValue()
{
    change_type(this, LIST_T);
}

TaggedValue*
List::append()
{
    return list_t::append((TaggedValue*) this);
}
void
List::append(TaggedValue* val)
{
    copy(val, append());
}

TaggedValue*
List::prepend()
{
    return list_t::prepend((TaggedValue*) this);
}

void
List::clear()
{
    list_t::clear((TaggedValue*) this);
}

int
List::length()
{
    return list_t::tv_num_elements((TaggedValue*) this);
}

bool
List::empty()
{
    return length() == 0;
}

TaggedValue*
List::get(int index)
{
    return list_t::tv_get_index((TaggedValue*) this, index);
}

void
List::set(int index, TaggedValue* value)
{
    list_t::tv_set_index((TaggedValue*) this, index, value);
}

void
List::resize(int newSize)
{
    list_t::resize(this, newSize); 
}

TaggedValue*
List::getLast()
{
    return get(length() - 1);
}

void
List::pop()
{
    resize(length() - 1);
}

void
List::removeNulls()
{
    list_t::remove_nulls(this);
}

List*
List::checkCast(TaggedValue* v)
{
    if (is_list(v))
        return (List*) v;
    else
        return NULL;
}

} // namespace circa
