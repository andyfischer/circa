// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace runtime_tests {

void blocked_by_error()
{
    Branch branch;

    internal_debug_function::spy_clear();

    branch.compile("test_spy(1)");
    Term *error = branch.compile("e = assert(false)");
    branch.compile("test_spy(2)");

    EvalContext context;
    evaluate_branch(&context, branch);
    test_assert(context.errorOccurred);
    test_assert(context.errorTerm == error);
    test_equals(internal_debug_function::spy_results(), "[1]");
}

void test_errored_function()
{
    Branch branch;
    branch.compile("e = assert(false)");
    Term* t = branch.compile("t = errored(e)");

    EvalContext context;
    evaluate_branch(&context, branch);

    test_assert(!context.errorOccurred);
    test_assert(as_bool(get_local(t)));
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(TYPE_TYPE->type == TYPE_TYPE);

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(FUNCTION_TYPE->type == TYPE_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_tests::blocked_by_error);
    REGISTER_TEST_CASE(runtime_tests::test_errored_function);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
}

} // namespace runtime_tests

} // namespace circa
