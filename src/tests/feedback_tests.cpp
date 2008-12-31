// Copyright 2008 Paul Hodge

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

    Term* apply_feedback = branch.eval("apply-feedback(a, 2)");

    if (apply_feedback->hasError()) {
        std::cout << apply_feedback->getErrorMessage() << std::endl;
        test_assert(false);
    }

    test_assert(as_int(a) == 2);

    // test string
    Term* str = branch.eval("str = \"sofa\"?");
    test_assert(as_string(str) == "sofa");
    branch.eval("apply-feedback(str, \"lamp\")");
    test_assert(as_string(str) == "lamp");
}

void test_add_feedback()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0?");
    branch.eval("sum = add(a,a)");
    branch.eval("apply-feedback(sum, 3.0)");

    test_assert(as_float(a) == 1.5);
}

} // namespace feedback_tests

void register_feedback_tests()
{
    REGISTER_TEST_CASE(feedback_tests::test_var_feedback);
    REGISTER_TEST_CASE(feedback_tests::test_add_feedback);
}

} // namespace circa
