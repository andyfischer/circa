// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace training_tests {

void test_simple()
{
    Branch branch;
    Term* a = branch.eval("a = 1.0");
    Term* b = branch.eval("b = add(a, 2.0)");

    a->boolProperty("trainable") = true;
    b->boolProperty("trainable") = true;

    Branch tbranch;
    generate_training(tbranch, b, tbranch.eval("1.0"));

}

void register_tests()
{
    REGISTER_TEST_CASE(test_simple);
}

} // namespace training_tests

} // namespace circa
