// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "testing.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void test_var_feedback()
{
    Branch branch;

    // test int
    Term* a = branch.eval("a = 1?");

    test_assert(as_int(a) == 1);

    test_assert(a->function == as_type(INT_TYPE).valueFunction);
    test_assert(as_function(a->function).feedbackPropogateFunction != NULL);

    Term* apply_feedback = branch.eval("apply_feedback(a, 2)");

    if (apply_feedback->hasError()) {
        std::cout << apply_feedback->getErrorMessage() << std::endl;
        test_assert(false);
    }

    test_assert(as_int(a) == 2);

    // test string
    Term* str = branch.eval("str = \"sofa\"?");
    test_assert(as_string(str) == "sofa");
    branch.eval("apply_feedback(str, \"lamp\")");
    test_assert(as_string(str) == "lamp");
}

void test_add_feedback()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0?");
    branch.eval("sum = add(a,a)");
    branch.eval("apply_feedback(sum, 3.0)");

    test_assert(as_float(a) == 1.5);
}

void register_tests()
{
    REGISTER_TEST_CASE(feedback_tests::test_var_feedback);
    REGISTER_TEST_CASE(feedback_tests::test_add_feedback);
}

} // namespace feedback_tests

} // namespace circa
