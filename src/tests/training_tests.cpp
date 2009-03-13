// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace training_tests {

void train_addition1()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    Term* b = branch.eval("b = add(a, 2.0)");

    a->boolProperty("trainable") = true;
    b->boolProperty("trainable") = true;

    Branch tbranch;
    generate_training(tbranch, b, tbranch.eval("4.0"));

    test_equals(as_float(a), 1.0);

    evaluate_branch(tbranch);

    test_equals(as_float(a), 2.0);
}

void train_addition2()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    Term* b = branch.eval("b = 2.0");
    Term* c = branch.eval("c = add(a, b)");

    a->boolProperty("trainable") = true;
    b->boolProperty("trainable") = true;
    c->boolProperty("trainable") = true;

    Branch tbranch;
    generate_training(tbranch, c, tbranch.eval("4.0"));

    evaluate_branch(tbranch);

    test_equals(as_float(a), 1.5);
    test_equals(as_float(b), 2.5);
}

void train_mult()
{
    Branch branch;
    Term* a = branch.eval("a = 2.0");
    /*Term* b =*/ branch.eval("b = 3.0");
    Term* c = branch.eval("c = mult(a, b)");

    a->boolProperty("trainable") = true;
    c->boolProperty("trainable") = true;

    Branch training;
    generate_training(training, c, training.eval("9.0"));
    evaluate_branch(training);

    test_equals(as_float(a), 3.0);
}

void register_tests()
{
    REGISTER_TEST_CASE(train_addition1);
    REGISTER_TEST_CASE(train_addition2);
    REGISTER_TEST_CASE(train_mult);
}

} // namespace training_tests

} // namespace circa
