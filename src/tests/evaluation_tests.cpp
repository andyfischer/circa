// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

void test_evaluate_with_lazy_stack()
{
    #if 0
    Branch branch;
    branch.compile("a = 1");
    branch.compile("b = 3");
    Term* c = branch.compile("add_i(a,a)");

    EvalContext context;
    evaluate_with_lazy_stack(&context, c);

    test_equals(context.stack.toString(), "[[1, null, 2]]");
    test_equals(as_int(c), 2);

    Term* d = branch.compile("add_i(a,b)");
    evaluate_with_lazy_stack(&context, d);
    test_equals(context.stack.toString(), "[[1, 3, 2, 4]]");
    test_equals(as_int(d), 4);
    #endif
}

void test_branch_eval()
{
    Branch branch;
    branch.eval("a = 1");
}

#if 0
void test_copy_stack_back_to_terms()
{
    Branch branch;
    Term* a = branch.compile("a = 3");
    Term* b = branch.compile("b = sqr(a)");
    branch.compile("c = 0; if true c = 1 end");
    Term* c = branch["c"];
    test_assert(c != NULL);

    // Artificially mess with registers
    a->registerIndex = 0;
    b->registerIndex = 1;
    c->registerIndex = 2;

    List frame;
    frame.resize(3);
    set_int(frame[0], 10);
    set_int(frame[1], 20);
    set_int(frame[2], 30);

    copy_stack_back_to_terms(branch, &frame);

    // Make sure value is not touched
    test_equals(a->asInt(), 3);

    test_equals(b->asInt(), 20);
    test_equals(c->asInt(), 30);
}
#endif

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_with_lazy_stack);
    REGISTER_TEST_CASE(evaluation_tests::test_branch_eval);
    //REGISTER_TEST_CASE(evaluation_tests::test_copy_stack_back_to_terms);
}

} // evaluation_tests
} // circa
