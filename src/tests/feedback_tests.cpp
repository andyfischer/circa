// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void test_var_feedback()
{
    Branch branch;

    // test int
    Term* a = eval_statement(branch, "a = 1");

    test_assert(as_int(a) == 1);

    Term* apply_feedback = eval_statement(branch, "apply-feedback(a, 2)");

    if (apply_feedback->hasError()) {
        std::cout << apply_feedback->getErrorMessage() << std::endl;
        test_assert(false);
    }

    test_assert(as_int(a) == 2);

    // test string
    Term* str = eval_statement(branch, "str = \"sofa\"");
    test_assert(as_string(str) == "sofa");
    eval_statement(branch, "apply-feedback(str, \"lamp\")");
    test_assert(as_string(str) == "lamp");
}

} // namespace feedback_tests

void register_feedback_tests()
{
    REGISTER_TEST_CASE(feedback_tests::test_var_feedback);
}

} // namespace circa
