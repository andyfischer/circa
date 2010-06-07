// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "builtin_types.h"
#include "debug_valid_objects.h"
#include "tagged_value.h"
#include "testing.h"

#include "type.h"

#include "list.h"

namespace circa {
namespace list_t {

bool is_list(TaggedValue* value);
void tv_mutate(TaggedValue* value);

///////// Internal data structure /////////
struct ListData {
    int refCount;
    int count;
    int capacity;
    TaggedValue items[0];
};

///////// Internally used functions /////////

// Increment refcount on this list
static void incref(ListData* data);

// Decrement refcount. This might cause this list to be deleted, don't
// use this data after you have given up your reference.
static void decref(ListData* data);

// Create a new list, starts off with 1 ref.
static ListData* create_list(int capacity);

// Creates a new list that is a duplicate of source. Starts off with 1 ref.
// Does not decref source.
static ListData* duplicate(ListData* source);

// Returns a new list with the given capacity. Decrefs original.
static ListData* increase_capacity(ListData* original, int new_capacity);

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
static ListData* grow_capacity(ListData* original);

// Modify a list so that it has the given number of elements, returns the new
// list data.
static ListData* resize(ListData* original, int numElements);

// Reset this list to have 0 elements.
static void clear(ListData** data);

// Return a version of this list which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
static ListData* mutate(ListData* data);

// Add a new blank element to the end of the list, resizing if necessary.
// Returns the new element.
static TaggedValue* append(ListData** data);

// Remove the element at index and replace the empty spot with the last
// element in the list.
static void remove_and_replace_with_back(ListData** data, int index);

void assert_valid_list(ListData* list)
{
    if (list == NULL) return;
    assert(list->refCount > 0);
    debug_assert_valid_object(list, LIST_OBJECT);
}

static void incref(ListData* data)
{
    assert_valid_list(data);
    data->refCount++;
}

static void decref(ListData* data)
{
    assert_valid_list(data);
    assert(data->refCount > 0);
    data->refCount--;

    if (data->refCount == 0) {
        // Release all elements
        for (int i=0; i < data->count; i++)
            make_null(&data->items[i]);
        free(data);
        debug_unregister_valid_object(data);
    }
}

static ListData* create_list(int capacity)
{
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
    debug_register_valid_object(result, LIST_OBJECT);

    result->refCount = 1;
    result->count = 0;
    result->capacity = capacity;
    for (int i=0; i < capacity; i++)
        result->items[i].init();
    return result;
}

static ListData* duplicate(ListData* source)
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

static ListData* increase_capacity(ListData* original, int new_capacity)
{
    if (original == NULL)
        return create_list(new_capacity);

    assert_valid_list(original);
    ListData* result = create_list(new_capacity);

    result->count = original->count;
    for (int i=0; i < result->count; i++)
        copy(&original->items[i], &result->items[i]);

    decref(original);
    return result;
}

static ListData* grow_capacity(ListData* original)
{
    if (original == NULL)
        return create_list(1);

    ListData* result = increase_capacity(original, original->capacity * 2);
    return result;
}

static ListData* resize(ListData* original, int numElements)
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
    ListData* result = mutate(original);

    // Possibly set extra elements to null, if we are shrinking.
    for (int i=numElements; i < result->count; i++)
        make_null(&result->items[i]);
    result->count = numElements;

    return result;
}

static void clear(ListData** data)
{
    if (*data == NULL) return;
    decref(*data);
    *data = NULL;
}

static ListData* mutate(ListData* data)
{
    if (data == NULL)
        return NULL;
    assert(data->refCount > 0);
    if (data->refCount == 1)
        return data;

    ListData* copy = duplicate(data);
    decref(data);
    return copy;
}

static TaggedValue* append(ListData** data)
{
    if (*data == NULL) {
        *data = create_list(1);
    } else {
        *data = mutate(*data);
        
        if ((*data)->count == (*data)->capacity)
            *data = grow_capacity(*data);
    }

    ListData* d = *data;
    d->count++;
    return &d->items[d->count - 1];
}

static void remove_and_replace_with_back(ListData** data, int index)
{
    *data = mutate(*data);
    assert(index < (*data)->count);

    make_null(&(*data)->items[index]);

    int lastElement = (*data)->count - 1;
    if (index < lastElement)
        swap(&(*data)->items[index], &(*data)->items[lastElement]);

    (*data)->count--;
}

static std::string to_string(ListData* value)
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

TaggedValue* append(TaggedValue* list)
{
    assert(is_list(list));
    return append((ListData**) &list->value_data);
}

void resize(TaggedValue* list, int newSize)
{
    assert(is_list(list));
    set_pointer(list, resize((ListData*) get_pointer(list), newSize));
}

void clear(TaggedValue* list)
{
    assert(is_list(list));
    clear((ListData**) &list->value_data);
}
void tv_initialize(Type* type, TaggedValue* value)
{
    set_pointer(value, NULL);

#ifdef NEWLIST
    // If type has a prototype then initialize to that.
    Branch& prototype = type->prototype;
    if (prototype.length() > 0) {
        List* list = (List*) value;
        list->resize(prototype.length());

        for (int i=0; i < prototype.length(); i++)
            change_type(list->get(i), type_contents(prototype[i]->type));
    }
#endif
}

void tv_release(TaggedValue* value)
{
    assert(is_list(value));
    ListData* data = (ListData*) get_pointer(value);
    assert_valid_list(data);
    if (data == NULL) return;
    decref(data);
}

void tv_copy(TaggedValue* source, TaggedValue* dest)
{
    assert(is_list(source));
    assert(is_list(dest));
    ListData* s = (ListData*) get_pointer(source);
    ListData* d = (ListData*) get_pointer(dest);

    assert_valid_list(s);
    assert_valid_list(d);

    set_pointer(dest, duplicate(s));

    if (s != NULL)
        incref(s);
    if (d != NULL)
        decref(d);
    
    set_pointer(dest, s);
}

bool tv_equals(TaggedValue* leftValue, TaggedValue* right)
{
    assert(is_list(leftValue));
    Type* rhsType = right->value_type;
    if (rhsType->numElements == NULL || rhsType->getIndex == NULL)
        return false;

    List* left = (List*) leftValue;

    int leftCount = left->numElements();

    if (leftCount != right->numElements())
        return false;

    for (int i=0; i < leftCount; i++) {
        if (!circa::equals(left->get(i), right->getIndex(i)))
            return false;
    }
    return true;
}

bool tv_cast_possible(Type*, TaggedValue* value)
{
    return is_list(value);
}

void tv_cast(Type*, TaggedValue* source, TaggedValue* dest)
{
    if (!is_list(source)) return;
    if (!is_list(dest)) return;

    bool keep_existing_shape = dest->value_type->prototype.length() != 0;

    if (keep_existing_shape) {
        List* s = (List*) source;
        List* d = (List*) dest;

        int count = std::min(s->numElements(), d->numElements());

        for (int i=0; i < count; i++)
            cast(s->get(i), d->get(i));

    } else {
        tv_copy(source, dest);
    }
}

TaggedValue* tv_get_index(TaggedValue* value, int index)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL) return NULL;
    if (index >= s->count) return NULL;
    return &s->items[index];
}

void tv_set_index(TaggedValue* value, int index, TaggedValue* element)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    assert(s);
    assert(s->count > index);

    tv_mutate(value);
    copy(element, &s->items[index]);
}

int tv_num_elements(TaggedValue* value)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL) return 0;
    return s->count;
}

std::string tv_to_string(TaggedValue* value)
{
    assert(is_list(value));
    return to_string((ListData*) get_pointer(value));
}

void tv_mutate(TaggedValue* value)
{
    assert(is_list(value));
    ListData* data = (ListData*) get_pointer(value);
    set_pointer(value, mutate(data));
}

void tv_static_type_query(Type* type, StaticTypeQuery* result)
{
    Term* term = result->targetTerm;
    Branch& prototype = type->prototype;
    
    // If prototype is empty then accept any list
    if (prototype.length() == 0)
        if (is_list_based_type(type_contents(term->type)))
            return result->succeed();
        else
            return result->fail();

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
    if (prototype.length() != numElements);

    for (int i=0; i < numElements; i++)
        if (!circa::value_fits_type(value->getIndex(i),
                    type_contents(prototype[i]->type)))
            return false;
    return true;
}

void remove_and_replace_with_back(TaggedValue* value, int index)
{
    assert(is_list(value));
    ListData* data = (ListData*) get_pointer(value);
    remove_and_replace_with_back(&data, index);
    set_pointer(value, data);
}

bool is_list(TaggedValue* value)
{
    return is_list_based_type(value->value_type);
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
    type->castPossible = tv_cast_possible;
    type->cast = tv_cast;
    type->getIndex = tv_get_index;
    type->setIndex = tv_set_index;
    type->numElements = tv_num_elements;
    type->mutate = tv_mutate;
    type->staticTypeQuery = tv_static_type_query;
    type->isSubtype = tv_is_subtype;
    type->valueFitsType = tv_value_fits_type;
}

void postponed_setup_type(Type*)
{
    // TODO: create member functions: append, count
}

int get_refcount(ListData* data)
{
    return data->refCount;
}

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
List::clear()
{
    list_t::clear((TaggedValue*) this);
}

int
List::length()
{
    return list_t::tv_num_elements((TaggedValue*) this);
}

TaggedValue*
List::get(int index)
{
    return list_t::tv_get_index((TaggedValue*) this, index);
}

void
List::resize(int newSize)
{
    list_t::resize(this, newSize); 
}

namespace list_t_tests {

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

        make_int(list_t::append(&value), 1);
        make_int(list_t::append(&value), 2);
        make_int(list_t::append(&value), 3);

        test_equals(to_string(&value), "[1, 2, 3]");

        test_assert(as_int(get_index(&value, 1)) == 2);
        test_assert(num_elements(&value) == 3);
    }

    void test_tagged_value_copy()
    {
        TypeRef list = Type::create();
        list_t::setup_type(list);

        TaggedValue value(list);

        make_int(list_t::append(&value), 1);
        make_int(list_t::append(&value), 2);
        make_int(list_t::append(&value), 3);

        test_equals(to_string(&value), "[1, 2, 3]");

        TaggedValue value2;
        test_assert(value.value_type->copy != NULL);
        copy(&value, &value2);

        test_equals(to_string(&value2), "[1, 2, 3]");

        make_int(list_t::append(&value2), 4);

        test_equals(to_string(&value), "[1, 2, 3]");
        test_equals(to_string(&value2), "[1, 2, 3, 4]");
    }

    void test_mutate()
    {
        TypeRef list = Type::create();
        list_t::setup_type(list);

        TaggedValue value(list);

        make_int(list_t::append(&value), 1);
        make_int(list_t::append(&value), 2);

        TaggedValue value2(list);
        copy(&value, &value2);

        test_assert(get_pointer(&value) == get_pointer(&value2));
        mutate(&value2);
        test_assert(get_pointer(&value) != get_pointer(&value2));
    }

    void register_tests()
    {
        REGISTER_TEST_CASE(list_t_tests::test_simple);
        REGISTER_TEST_CASE(list_t_tests::test_tagged_value);
        REGISTER_TEST_CASE(list_t_tests::test_tagged_value_copy);
        REGISTER_TEST_CASE(list_t_tests::test_mutate);
    }

} // namespace list_t_tests

}
