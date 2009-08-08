// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
    Term* t = branch.eval("1 + 2");
    test_assert(get_static_error(t) == SERROR_NO_ERROR);
    test_assert(!has_static_error(t));
}

void test_unknown_func()
{
#if 0
TODO: This code actually gives an unknown-identifier error

    Branch branch;
    Term* t = branch.eval("embiggen(1)");
    test_equals(get_static_error_message(t), "Unknown function: embiggen");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_FUNCTION);
#endif
}

void test_unknown_type()
{
    Branch branch;
    branch.eval("type T { X x }");
    Term* t = branch[0];
    test_assert(t->name == "X");
    test_assert(t->function == UNKNOWN_TYPE_FUNC);
    test_equals(get_static_error_message(t), "Unknown type: X");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_TYPE);
}

void test_unknown_identifier()
{
    Branch branch;
    Term* t = branch.eval("charlie");
    test_equals(get_static_error_message(t), "Unknown identifier: charlie");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_IDENTIFIER);

    branch.eval("a = 1");
    t = branch.eval("a.b");
    test_equals(get_static_error_message(t), "Unknown identifier: a.b");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_IDENTIFIER);
}

void register_tests()
{
    REGISTER_TEST_CASE(static_errors_tests::input_type_error);
    REGISTER_TEST_CASE(static_errors_tests::no_error);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_func);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_type);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_identifier);
}

} // namespace static_errors_tests

} // namespace circa
