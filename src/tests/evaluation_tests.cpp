// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

namespace circa {
namespace evaluation_tests {

void test_lazy()
{
#ifdef DEFERRED_CALLS_FIRST_DRAFT
    Branch branch;

    Term* a = branch.compile("a = add(1 2)");
    set_lazy_call(a, true);

    Stack context;
    push_frame(&context, &branch);
    run_interpreter(&context);

    test_equals(get_register(&context, a), ":Unevaluated");

    Frame* frame = push_frame(&context, &branch);
    frame->pc = a->index;
    frame->startPc = frame->pc;
    frame->endPc = frame->pc + 1;
    frame->strategy = ByDemand;
    run_interpreter(&context);

    test_equals(get_register(&context, a), "3");

    reset_stack(&context);
    Term* b = branch.compile("b = add(a a)");
    push_frame(&context, &branch);
    run_interpreter(&context);

    test_equals(get_register(&context, a), "3");
    test_equals(get_register(&context, b), "6");
#endif
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_lazy);
}

} // namespace evaluation_tests
} // namespace circa
