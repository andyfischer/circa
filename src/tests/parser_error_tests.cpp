// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace parser_error_tests {

void call_unknown_function()
{
    Branch branch;
    branch.compile("blah()");

    test_assert(has_static_errors(&branch));
}

void call_unknown_namespaced_function()
{
    Branch branch;
    branch.compile("blee:blah()");

    test_assert(has_static_errors(&branch));
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_function);
    REGISTER_TEST_CASE(parser_error_tests::call_unknown_namespaced_function);
}

} // namespace parser_error_tests
} // namespace circa
