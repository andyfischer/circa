// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "string_type.h"

namespace string_test {

void test_sneaky_equals()
{
    Value val1, val2;
    set_string(&val1, "Hello");
    set_string(&val2, "Hello");

    // initial: strings are stored differently.
    test_assert(circa_string(&val1) != circa_string(&val2));

    test_assert(circa_equals(&val1, &val2));

    // after equality check: strings are stored with same object.
    test_assert(circa_string(&val1) == circa_string(&val2));
}

void test_prepend()
{
    Value hello, there;
    set_string(&hello, "hello");
    set_string(&there, "there");

    test_equals(&hello, "hello");
    test_equals(&there, "there");

    string_prepend(&there, " ");
    test_equals(&there, " there");
    string_prepend(&there, &hello);
    string_prepend(&there, "hello there");
    test_equals(&hello, "hello");
}

void register_tests()
{
    REGISTER_TEST_CASE(string_test::test_sneaky_equals);
    REGISTER_TEST_CASE(string_test::test_prepend);
}

} // namespace string_test
