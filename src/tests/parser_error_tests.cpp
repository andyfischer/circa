// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace parser_error_tests {

void call_unknown_function()
{
    Branch branch;
    branch.compile("blah()");

    StaticErrorCheck result;

    check_for_static_errors(&result, branch);

    test_equals(result.count(), 1);
}

void call_unknown_namespaced_function()
{
    Branch branch;
    branch.compile("blee:blah()");

    StaticErrorCheck result;

    check_for_static_errors(&result, branch);

    test_equals(result.count(), 1);
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_function);
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_namespaced_function);
}

} // namespace parser_error_tests
} // namespace circa
