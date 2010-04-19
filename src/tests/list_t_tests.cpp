// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
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
    test_assert(get_element(&value, 1) == NULL);
    test_assert(num_elements(&value) == 0);

    make_int(list_t::append(&value), 1);
    make_int(list_t::append(&value), 2);
    make_int(list_t::append(&value), 3);

    test_equals(to_string(&value), "[1, 2, 3]");

    test_assert(as_int(get_element(&value, 1)) == 2);
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

}
}
