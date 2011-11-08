// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

#include "list_shared.h"

namespace circa {
namespace list_tests {

void test_parse_type()
{
    Branch branch;

    Term* myType = branch.compile("type MyType { string s, int i }");

    // Check MyType's parameter
    test_equals(&as_type(myType)->parameter, "[[<Type string>, <Type int>], ['s', 'i']]");

    // Check an instanciated value
    TaggedValue* val = branch.eval("MyType()");
    test_assert(is_string(val->getIndex(0)));
    test_assert(is_int(val->getIndex(1)));

    test_assert(is_string(val->getField("s")));
    test_assert(is_int(val->getField("i")));
    test_assert(val->getField("x") == NULL);
}

void test_cpp_wrapper_simple()
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

void test_cast()
{
    Branch branch;

    TaggedValue a;
    set_list(&a);

    TaggedValue b;
    set_list(&b, 2);
    set_int(b.getIndex(0), 1);
    set_int(b.getIndex(1), 2);
    test_equals(b.toString(), "[1, 2]");

    TaggedValue c;
    set_list(&c, 2);
    set_int(c.getIndex(0), 1);
    set_string(c.getIndex(1), "hi");
    test_equals(c.toString(), "[1, 'hi']");

    TaggedValue d;
    set_list(&d, 1);
    set_float(d.getIndex(0), 1);
    test_equals(d.toString(), "[1.0]");

    TaggedValue x;

    test_assert(!cast_possible(&a, &INT_T));
    test_assert(cast_possible(&a, &LIST_T));

    test_assert(cast_possible(&b, &LIST_T));
    test_assert(cast(&b, &LIST_T, &x));
    test_equals(x.toString(), "[1, 2]");

    Term* t_term = branch.compile("type T { int i, number f }");
    Type* t = unbox_type(t_term);

    test_assert(!cast_possible(&a, t));
    test_assert(cast_possible(&b, t));
    test_assert(!cast_possible(&c, t));
    test_assert(!cast_possible(&d, t));

    test_assert(cast(&b, t, &x));
    test_equals(x.toString(), "[1, 2.0]");
}

void test_remove_nulls()
{
    TaggedValue v;
    List list;
    test_equals(list.toString(), "[]");

    for (int i=0; i < 6; i++) {
        set_int(&v, i);
        list.append(&v);
    }

    test_equals(list.toString(), "[0, 1, 2, 3, 4, 5]");

    list.removeNulls();
    test_equals(list.toString(), "[0, 1, 2, 3, 4, 5]");

    set_null(list[3]);
    test_equals(list.toString(), "[0, 1, 2, null, 4, 5]");

    list.removeNulls();
    test_equals(list.toString(), "[0, 1, 2, 4, 5]");

    set_null(list[4]);
    list.removeNulls();
    test_equals(list.toString(), "[0, 1, 2, 4]");

    set_null(list[0]);
    set_null(list[1]);
    set_null(list[3]);
    list.removeNulls();
    test_equals(list.toString(), "[2]");

    set_null(list[0]);
    list.removeNulls();
    test_equals(list.toString(), "[]");
}

void test_remove_index()
{
    List list;
    set_int(list.append(), 0);
    set_int(list.append(), 1);
    set_int(list.append(), 2);
    set_int(list.append(), 3);

    test_equals(&list, "[0, 1, 2, 3]");
    list.remove(2);
    test_equals(&list, "[0, 1, 3]");
    list.remove(0);
    test_equals(&list, "[1, 3]");
    list.remove(1);
    test_equals(&list, "[1]");
    list.remove(0);
    test_equals(&list, "[]");
}


void test_tagged_value()
{
    Type* list = create_type();
    list_t::setup_type(list);

    TaggedValue value;
    create(list, &value);

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
    Type* list = create_type();
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
    Type* list = create_type();
    list_t::setup_type(list);

    TaggedValue value(list);

    set_int(list_t::append(&value), 1);
    set_int(list_t::append(&value), 2);

    TaggedValue value2(list);
    copy(&value, &value2);

    #if !CIRCA_DISABLE_LIST_SHARING
    test_assert(get_pointer(&value) == get_pointer(&value2));
    #endif
    touch(&value2);
    test_assert(get_pointer(&value) != get_pointer(&value2));
}

void test_prepend()
{
    Type* list = create_type();
    list_t::setup_type(list);

    TaggedValue value(list);

    set_int(list_t::append(&value), 1);
    set_int(list_t::append(&value), 2);

    test_assert(to_string(&value) == "[1, 2]");
    list_t::prepend(&value);
    test_assert(to_string(&value) == "[null, 1, 2]");
    set_int(list_get_index(&value, 0), 4);
    test_assert(to_string(&value) == "[4, 1, 2]");

    reset(&value);

    test_assert(to_string(&value) == "[]");
    list_t::prepend(&value);
    test_assert(to_string(&value) == "[null]");

    reset(&value);

    list_t::prepend(&value);
    set_int(list_get_index(&value, 0), 1);
    test_assert(to_string(&value) == "[1]");
    list_t::prepend(&value);
    test_assert(to_string(&value) == "[null, 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(list_tests::test_parse_type);
    REGISTER_TEST_CASE(list_tests::test_cpp_wrapper_simple);
    REGISTER_TEST_CASE(list_tests::test_cast);
    REGISTER_TEST_CASE(list_tests::test_remove_nulls);
    REGISTER_TEST_CASE(list_tests::test_remove_index);
    REGISTER_TEST_CASE(list_tests::test_tagged_value);
    REGISTER_TEST_CASE(list_tests::test_tagged_value_copy);
    REGISTER_TEST_CASE(list_tests::test_touch);
    REGISTER_TEST_CASE(list_tests::test_prepend);
}

}
} // namespace circa
