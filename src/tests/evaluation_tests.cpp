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

void test_evaluate_minimum()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = 2");
    Term* c = branch.compile("c = add(a b)");
    Term* d = branch.compile("d = sub(a b)");

    test_equals(get_local(a), "null");
    test_equals(get_local(b), "null");
    test_equals(get_local(c), "null");
    test_equals(get_local(d), "null");

    EvalContext context;
    evaluate_minimum(&context, d);

    test_equals(get_local(a), "1");
    test_equals(get_local(b), "2");
    test_equals(get_local(c), "null");
    test_equals(get_local(d), "-1");
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_with_lazy_stack);
    REGISTER_TEST_CASE(evaluation_tests::test_branch_eval);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum);
}

} // evaluation_tests
} // circa
