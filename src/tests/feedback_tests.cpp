// Copyright 2008 Andrew Fischer

#include "common_headers.h"
#include "testing.h"
#include "circa.h"

namespace circa {
namespace feedback_tests {

void train_addition1()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    branch.eval("b = add(a, 2.0)");
    branch.eval("b <- 4");

    set_trainable(a, true);

    test_equals(as_float(a), 1.0);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(as_float(a), 2.0);
}

void train_addition2()
{
    // In this test, we split a feedback signal to two trainable values
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    Term* b = branch.eval("b = 2.0");
    branch.eval("add(a, b) <- 4");

    set_trainable(a, true);
    set_trainable(b, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(to_float(a), 1.5);
    test_equals(to_float(b), 2.5);
}

void train_mult()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    branch.eval("b = 3");
    branch.eval("mult(a, b) <- 6");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(to_float(a), 2.0);
}

void train_if_expr()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    branch.eval("b = 1");
    Term* cond = branch.eval("cond = true");
    branch.eval("if_expr(cond, a, b) <- 5");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_assert(a->asInt() == 5);

    // try changing cond
    a->asInt() = 1;
    cond->asBool() = false;

    evaluate_branch(branch);

    test_assert(a->asInt() == 1);
}

void train_sin()
{
    Branch branch;
    Term* a = branch.eval("a = 0.0");
    branch.eval("sin(a) <- 1");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(as_float(a), M_PI / 2.0);
}

void train_cos()
{
    Branch branch;
    Term* a = branch.eval("a = 5.0");
    branch.eval("cos(a) <- 1");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(as_float(a), 0);
}

void feedback_across_function()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    branch.eval("def inv(float f) : float\nreturn 0 - f\nend");
    branch.eval("b = inv(a)");
    branch.eval("b <- -2.0");

    set_trainable(a, true);
    refresh_training_branch(branch);
}

void feedback_operation()
{
    FeedbackOperation operation;
    Branch branch;
    Term* a = branch.eval("1");
    Term* b = branch.eval("2");

    RefList list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.length() == 0);

    operation.sendFeedback(a, b, DESIRED_VALUE_FEEDBACK);

    list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.length() == 1);
    test_assert(list[0] == b);
}

void register_tests()
{
    REGISTER_TEST_CASE(feedback_tests::train_addition1);
    REGISTER_TEST_CASE(feedback_tests::train_addition2);
    REGISTER_TEST_CASE(feedback_tests::train_mult);
    REGISTER_TEST_CASE(feedback_tests::train_if_expr);
    REGISTER_TEST_CASE(feedback_tests::train_sin);
    REGISTER_TEST_CASE(feedback_tests::train_cos);
    REGISTER_TEST_CASE(feedback_tests::feedback_operation);
}

} // namespace feedback_tests

} // namespace circa
