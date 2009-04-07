// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace stateful_tests {

void stateful_value_evaluation()
{
    Branch branch;
    Term *i = branch.eval("state float i = 2.0");
    branch.eval("i = i + 1.0");
    parser::wrap_up_branch(branch);

    test_equals(as_float(i), 2.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 3.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 4.0);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_tests::stateful_value_evaluation);
}

} // namespace stateful_tests

} // namespace circa
