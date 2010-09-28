// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace evaluation_tests {

void test_evaluate_in_place()
{
    Branch branch;
    branch.compile("a = 1");
    branch.compile("b = 1");
    Term* c = branch.compile("add_i(a,b)");
    test_assert(as_int(c) == 0);

    //evaluate_in_place(c);
    test_assert(as_int(c) == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(evaluation_tests::test_evaluate_in_place);
}

} // evaluation_tests
} // circa
