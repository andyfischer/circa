// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace foreach_type_tests {

bool run_test_for_type(Term* type, List& exampleValues)
{
    Branch branch;

    Term* x = create_value(branch, type, "x");
    assign_value(exampleValues[0], x);

    test_assert(branch.eval("x == x"));

    Term* cpy = branch.eval("cpy = copy(x)");
    test_assert(branch);
    test_assert(equals(x, cpy));

    Term* y = create_value(branch, type, "y");
    assign_value(exampleValues[1], x);
    test_assert(!equals(x,y));
    test_assert(branch.eval("x != y"));

    Term* cnd1 = branch.eval("cond(true, x, y)");
    Term* cnd2 = branch.eval("cond(false, x, y)");
    test_assert(branch);
    test_assert(equals(cnd1, x));
    test_assert(equals(cnd2, y));

    branch.eval("boxed = [x y]");
    Term* unbox1 = branch.eval("boxed[0]");
    Term* unbox2 = branch.eval("boxed[1]");
    test_assert(branch);
    test_assert(equals(unbox1, x));
    test_assert(equals(unbox2, y));

    return true;
}

void run()
{
    List intExamples;
    make_int(intExamples.append(), 5);
    make_int(intExamples.append(), 3);
    run_test_for_type(INT_TYPE, intExamples);

    List floatExamples;
    make_float(floatExamples.append(), 1.2);
    make_float(floatExamples.append(), -0.001);
    run_test_for_type(FLOAT_TYPE, floatExamples);

    List stringExamples;
    make_string(stringExamples.append(), "hello");
    make_string(stringExamples.append(), "goodbye");
    run_test_for_type(STRING_TYPE, stringExamples);

    List refExamples;
    Term* refTarget1 = alloc_term();
    Term* refTarget2 = alloc_term();
    make_ref(refExamples.append(), refTarget1);
    make_ref(refExamples.append(), refTarget2);
    run_test_for_type(REF_TYPE, refExamples);
}

void register_tests()
{
    REGISTER_TEST_CASE(foreach_type_tests::run);
}

}
}
