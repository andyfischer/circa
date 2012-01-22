// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace evaluation_tests {

void test_lazy()
{
    Branch branch;

    Term* a = branch.compile("a = add(1 2)");
    set_lazy_call(a, true);

    EvalContext context;
    push_frame(&context, &branch);
    run_interpreter(&context);
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_lazy);
}

} // namespace evaluation_tests
} // namespace circa
