// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace runtime_error_tests {

void test_error_message()
{
    Branch branch;
    branch.compile("assert(false)");
    EvalContext context;
    evaluate_branch(&context, &branch);
    test_assert(context.errorOccurred);
    test_equals(context_get_error_message(&context), "Assert failed");
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_error_tests::test_error_message);
}


} // namespace runtime_error_tests
} // namespace circa
