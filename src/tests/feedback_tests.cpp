// Copyright 2008 Paul Hodge

#include "common_headers.h"
#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void test_no_feedback_function()
{
    Branch branch;




}

} // namespace feedback_tests

void register_feedback_tests()
{
    REGISTER_TEST_CASE(feedback_tests::test_no_feedback_function);
}

} // namespace circa
