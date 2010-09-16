// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "build_options.h"
#include "builtin_types.h"
#include "debug_valid_objects.h"
#include "tagged_value.h"
#include "tvvector.h"
#include "testing.h"

#include "type.h"

#include "list.h"

using namespace circa::tvvector;

namespace circa {
namespace list_t {

bool is_list(TaggedValue* value);
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
    set_pointer(dest, tvvector::duplicate(s));
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

void tv_cast(Type*, TaggedValue* source, TaggedValue* dest)
{
    // FIXME: should these be asserts?
    if (!is_list(source)) return;
    if (!is_list(dest)) return;

    bool keep_existing_shape = dest->value_type->prototype.length() != 0;

    if (keep_existing_shape) {
        List* s = List::checkCast(source);
        List* d = List::checkCast(dest);

        int count = std::min(s->numElements(), d->numElements());

        for (int i=0; i < count; i++)
            cast(s->get(i), d->get(i));

    } else {
        tv_copy(source, dest);
    }
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
    make_list(OUTPUT);
    List* result = List::checkCast(OUTPUT);
    copy(INPUT(0), OUTPUT);
    TaggedValue* value = INPUT(1);
    copy(value, result->append());
}

CA_FUNCTION(count)
{
    List* list = List::checkCast(INPUT(0));
    make_int(OUTPUT, list->length());
}

void postponed_setup_type(Term* type)
{
    Term* list_append =
        import_member_function(type, append, "append(List, any) -> List");
    function_set_use_input_as_output(list_append, 0, true);
    import_member_function(type, count, "count(List) -> int");
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

List*
List::checkCast(TaggedValue* v)
{
    if (list_t::is_list(v))
        return (List*) v;
    else
        return NULL;
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

    void test_touch()
    {
        TypeRef list = Type::create();
        list_t::setup_type(list);

        TaggedValue value(list);

        make_int(list_t::append(&value), 1);
        make_int(list_t::append(&value), 2);

        TaggedValue value2(list);
        copy(&value, &value2);

        test_assert(get_pointer(&value) == get_pointer(&value2));
        touch(&value2);
        test_assert(get_pointer(&value) != get_pointer(&value2));
    }

    void register_tests()
    {
        REGISTER_TEST_CASE(list_t_tests::test_simple);
        REGISTER_TEST_CASE(list_t_tests::test_tagged_value);
        REGISTER_TEST_CASE(list_t_tests::test_tagged_value_copy);
        REGISTER_TEST_CASE(list_t_tests::test_touch);
    }

} // namespace list_t_tests

}
