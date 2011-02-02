// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace parser_error_tests {

void call_unknown_function()
{
    Branch branch;
    branch.compile("blah()");

    StaticErrorCheck result;

    check_for_static_errors(&result, branch);

    test_assert(!result.empty());
}

void call_unknown_namespaced_function()
{
    Branch branch;
    branch.compile("blee:blah()");

    StaticErrorCheck result;

    check_for_static_errors(&result, branch);

    test_assert(!result.empty());
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_function);
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_namespaced_function);
}

} // namespace parser_error_tests
} // namespace circa
