// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void test_no_feedback_function()
{
    Branch branch;

    Term* a = eval_statement(branch, "a = 1?");

    Term* apply_feedback = eval_statement(branch, "apply-feedback(a, 2)");


}

} // namespace feedback_tests

void register_feedback_tests()
{
    REGISTER_TEST_CASE(feedback_tests::test_no_feedback_function);
}

} // namespace circa
