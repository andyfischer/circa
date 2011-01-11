// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

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
    REGISTER_TEST_CASE(evaluation_tests::test_branch_eval);
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_minimum);
}

} // evaluation_tests
} // circa
