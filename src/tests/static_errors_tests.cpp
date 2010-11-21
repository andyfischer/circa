// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace static_errors_tests {

void input_type_error()
{
    Branch branch;
    Term* t = branch.eval("add_f('hi', 'bye')");

    test_assert(get_static_error(t) == SERROR_INPUT_TYPE_ERROR);
    test_assert(has_static_error(t));
}

void no_error()
{
    Branch branch;
    Term* t = branch.compile("1 + 2");
    test_assert(get_static_error(t) == SERROR_NO_ERROR);
    test_assert(!has_static_error(t));
}

void test_unknown_func()
{
    Branch branch;
    Term* t = branch.compile("embiggen(1)");
    test_equals(get_static_error_message(t), "Unknown function: embiggen");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_FUNCTION);
}

void test_unknown_type()
{
    Branch branch;
    branch.compile("type T { X x }");
    Term* t = branch[0];
    test_assert(t->name == "X");
    test_assert(t->function == UNKNOWN_TYPE_FUNC);
    test_equals(get_static_error_message(t), "Unknown type: X");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_TYPE);
    test_assert(has_static_errors(branch));
}

void test_unknown_identifier()
{
    Branch branch;
    Term* t = branch.eval("charlie");
    test_equals(get_static_error_message(t), "Unknown identifier: charlie");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_IDENTIFIER);
    test_assert(has_static_error(t));
    test_assert(has_static_errors(branch));

    branch.clear();
    t = branch.eval("a:b");
    #if 0
    TEST_DISABLED
    test_equals(get_static_error_message(t), "Unknown identifier: a:b");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_IDENTIFIER);
    test_assert(has_static_errors(branch));
    #endif
}

void register_tests()
{
    //TEST_DISABLED REGISTER_TEST_CASE(static_errors_tests::input_type_error);
    REGISTER_TEST_CASE(static_errors_tests::no_error);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_func);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_type);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_identifier);
}

} // namespace static_errors_tests

} // namespace circa
