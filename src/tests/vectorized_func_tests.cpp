// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace vectorized_func_tests {

void test_type_inference()
{
    Branch branch;
    Term* t = branch.compile("[1 1]*[1 1]");

    test_assert(nested_contents(t)->length() > 0);

    // make sure there is a mult_i in there somewhere
    bool found_mult_i = false;
    for (BranchIterator it(&branch); it.unfinished(); it.advance())
    {
        if (it->function->name == "mult_i")
            found_mult_i = true;
        test_assert(it->function->name != "mult_f");
    }

    evaluate_branch(&branch);
    test_equals(get_local(t), "[1, 1]");
}

void register_tests()
{
    REGISTER_TEST_CASE(vectorized_func_tests::test_type_inference);
}

} // namespace vectorized_func_tests
} // namespace circa
