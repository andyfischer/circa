// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace stateful_tests {

void stateful_value_evaluation()
{
    Branch branch;
    Term *i = branch.eval("state i = 2.0");
    branch.eval("i = i + 1.0");
    wrap_up_branch(branch);

    test_equals(as_float(i), 2.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 3.0);
    evaluate_branch(branch);
    test_equals(as_float(i), 4.0);
}

void initialize_from_expression()
{
    Branch branch;
    branch.eval("a = 1 + 2");
    branch.eval("b = a * 2");
    Term *c = branch.eval("state c = b");

    test_equals(as_float(c), 6);
}

void register_tests()
{
    REGISTER_TEST_CASE(stateful_tests::stateful_value_evaluation);
    REGISTER_TEST_CASE(stateful_tests::initialize_from_expression);
}

} // namespace stateful_tests

} // namespace circa
