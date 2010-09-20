// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

void test_evaluate_single_term()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("b = 2");
    Term* c = branch.compile("c = add(a,b)");

    test_assert(as_int(c) == 0);

    evaluate_single_term(c);

    test_assert(as_int(c) == 3);
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_single_term);
}

} // evaluation_tests
} // circa
