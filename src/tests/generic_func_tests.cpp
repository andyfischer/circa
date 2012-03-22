// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circ_internal.h"
#include "building.h"
#include "generic.h"

namespace circa {
namespace generic_func_tests {

void test_vectorize_vs()
{
    Branch branch;
    Term* myFunc = create_function(&branch, "myfunc");
    create_function_vectorized_vs(function_contents(myFunc), FUNCS.add_i, &LIST_T, &INT_T);

    Term* call = branch.compile("myfunc([1 2 3], 4)");

    test_assert(&branch);

    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_equals(call, "[5, 6, 7]");
}

void test_vectorize_vv()
{
    Branch branch;
    Term* myFunc = create_function(&branch, "myfunc");
    create_function_vectorized_vv(function_contents(myFunc), FUNCS.add_i, &LIST_T, &LIST_T);

    Term* call = branch.compile("myfunc([1 2 3] [2 2 3])");

    test_assert(&branch);

    EvalContext context;
    evaluate_save_locals(&context, &branch);
    test_equals(call, "[3, 4, 6]");
}

void test_overloaded()
{
}

void register_tests()
{
    REGISTER_TEST_CASE(generic_func_tests::test_vectorize_vs);
    REGISTER_TEST_CASE(generic_func_tests::test_vectorize_vv);
    REGISTER_TEST_CASE(generic_func_tests::test_overloaded);
}

}
} // namespace circa
