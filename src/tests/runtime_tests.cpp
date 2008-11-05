// Copyright 2008 Paul Hodge

#include "tests/common.h"
#include "branch.h"
#include "builtins.h"
#include "introspection.h"
#include "parser.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "ref_list.h"
#include "values.h"

namespace circa {
namespace runtime_tests {

void safe_delete()
{
    Branch branch;

    Term* term1 = apply_statement(branch, "term1 = 1");
    Term* termSum = apply_statement(branch, "sum = add(term1,2)");

    test_assert(branch.containsName("term1"));
    test_assert(branch.containsName("sum"));
    test_assert(termSum->inputs[0] == term1);

    delete term1;

    test_assert(termSum->inputs[0] == NULL);
    test_assert(!branch.containsName("term1"));

    for (int i=0; i < branch.numTerms(); i++)
    {
        if (branch[i] == term1)
            test_fail();
    }
}

void test_create_var()
{
    Branch branch;
    Term *term = create_var(&branch, INT_TYPE);

    test_assert(term->type == INT_TYPE);
}

void test_int_var()
{
    Branch branch;
    Term *term = int_var(branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = int_var(branch, 154, "george");
    test_assert(term2 == branch.getNamed("george"));
    test_assert(term2->name == "george");
    test_assert(as_int(term2) == 154);
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(TYPE_TYPE->type == TYPE_TYPE);

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(FUNCTION_TYPE->type == TYPE_TYPE);
}

void test_find_existing_equivalent()
{
    Branch branch;

    Term* add_func = eval_statement(branch, "add");
    Term* a = eval_statement(branch, "a = 1");
    Term* b = eval_statement(branch, "b = 1");
    Term* addition = eval_statement(branch, "add(a,b)");

    test_assert(is_equivalent(addition, add_func, ReferenceList(a,b)));

    test_assert(addition == find_equivalent(add_func, ReferenceList(a,b)));

    test_assert(NULL == find_equivalent(add_func, ReferenceList(b,a)));

    Term* another_addition = eval_statement(branch, "add(a,b)");

    test_assert(addition == another_addition);

    Term* a_different_addition = eval_statement(branch, "add(b,a)");

    test_assert(addition != a_different_addition);
}

void var_function_reuse()
{
    Branch branch;

    Term* function = get_var_function(branch, INT_TYPE);
    Term* function2 = get_var_function(branch, INT_TYPE);

    test_assert(function == function2);

    Term* a = int_var(branch, 3);
    Term* b = int_var(branch, 4);

    test_assert(a->function == b->function);
}

} // namespace runtime_tests

void register_runtime_tests()
{
    REGISTER_TEST_CASE(runtime_tests::safe_delete);
    REGISTER_TEST_CASE(runtime_tests::test_create_var);
    REGISTER_TEST_CASE(runtime_tests::test_int_var);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::test_find_existing_equivalent);
    REGISTER_TEST_CASE(runtime_tests::var_function_reuse);
}

} // namespace circa
