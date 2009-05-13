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
    branch.eval("feedback(b, 4)");

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
    branch.eval("c = add(a, b)");
    branch.eval("feedback(c, 4)");

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
    branch.eval("c = mult(a, b)");
    branch.eval("feedback(c, 6)");

    set_trainable(a, true);

    refresh_training_branch(branch);
    evaluate_branch(branch);

    test_equals(to_float(a), 2.0);
}

void train_sin()
{
    Branch branch;
    Term* a = branch.eval("a = 0.0");
    Term* b = branch.eval("b = sin(a)");

    a->boolProp("trainable") = true;
    b->boolProp("trainable") = true;

    Branch training;
    //generate_feedback(training, b, training.eval("1.0"));
    evaluate_branch(training);
}

void feedback_across_function()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    branch.eval("def inv(float f) : float\nreturn 0 - f\nend");
    branch.eval("b = inv(a)");
    branch.eval("feedback(b, -2.0)");

    set_trainable(a, true);
    refresh_training_branch(branch);
}

void feedback_operation()
{
    /* FIXME
    FeedbackOperation operation;
    Branch branch;
    Term* a = branch.eval("1");
    Term* b = branch.eval("2");

    RefList list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.count() == 0);

    operation.sendFeedback(a, b, DESIRED_VALUE_FEEDBACK);

    list = operation.getFeedback(a, DESIRED_VALUE_FEEDBACK);

    test_assert(list.count() == 1);
    test_assert(list[0] == b);
    */
}

void register_tests()
{
    REGISTER_TEST_CASE(feedback_tests::train_addition1);
    REGISTER_TEST_CASE(feedback_tests::train_addition2);
    REGISTER_TEST_CASE(feedback_tests::train_mult);
    //REGISTER_TEST_CASE(feedback_tests::train_sin);
    //FIXME REGISTER_TEST_CASE(feedback_tests::test_refresh_training_branch);
    //FIXME REGISTER_TEST_CASE(feedback_tests::test_refresh_training_branch2);
    REGISTER_TEST_CASE(feedback_tests::feedback_operation);
}

} // namespace feedback_tests

} // namespace circa
