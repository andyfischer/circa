// Copyright 2009 Paul Hodge

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
    Branch branch;
    Term* t = branch.eval("embiggen(1)");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_FUNCTION);
    test_equals(get_static_error_message(t), "Unknown function: embiggen");
}

void test_unknown_type()
{
    // TODO
#if 0
    Branch branch;
    Term* t = branch.eval("i = 2 : RhinosPerCubicSecond");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_TYPE);
    test_equals(get_static_error_message(t), "Unknown type: RhinosPerCubicSecond");
#endif
}

void test_unknown_identifier()
{
    Branch branch;
    Term* t = branch.eval("charlie");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_IDENTIFIER);
    test_equals(get_static_error_message(t), "Unknown identifier: charlie");
}

void test_unknown_field()
{
    Branch branch;
    branch.eval("a = 1");
    Term* t = branch.eval("a.b");
    test_assert(get_static_error(t) == SERROR_UNKNOWN_FIELD);
    test_equals(get_static_error_message(t), "Unknown field: b");
}

void register_tests()
{
    REGISTER_TEST_CASE(static_errors_tests::input_type_error);
    REGISTER_TEST_CASE(static_errors_tests::no_error);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_func);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_type);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_identifier);
    REGISTER_TEST_CASE(static_errors_tests::test_unknown_field);
}

} // namespace static_errors_tests

} // namespace circa
