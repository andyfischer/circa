// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "builtins.h"
#include "cpp_interface.h"
#include "introspection.h"
#include "parser.h"
#include "runtime.h"
#include "testing.h"
#include "term.h"
#include "type.h"
#include "ref_list.h"
#include "values.h"

namespace circa {
namespace runtime_tests {

void test_create_value()
{
    Branch branch;
    Term *term = create_value(&branch, INT_TYPE);
    test_assert(term->type == INT_TYPE);
    test_assert(term->value != NULL);

    term = create_value(&branch, BRANCH_TYPE);
    test_assert(term->value != NULL);
    test_assert(as_branch(term).owningTerm == term);
}

void test_int_value()
{
    Branch branch;
    Term *term = int_value(branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = int_value(branch, 154, "george");
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

    Term* a = int_value(branch, 3);
    Term* b = int_value(branch, 4);

    test_assert(a->function == b->function);
}

void null_input_errors()
{
    Branch branch;

    Term* one = float_value(branch, 1.0);

    Term* term1 = apply_function(branch, get_global("add"), ReferenceList(NULL, one));
    evaluate_term(term1);
    test_assert(term1->hasError());
    test_assert(term1->getErrorMessage() == "Input 0 is NULL");

    term1->function = NULL;
    evaluate_term(term1);
    test_assert(term1->hasError());
    test_assert(term1->getErrorMessage() == "Function is NULL");

    Term* term2 = apply_function(branch, get_global("add"), ReferenceList(one, NULL));
    evaluate_term(term2);
    test_assert(term2->hasError());
    test_assert(term2->getErrorMessage() == "Input 1 is NULL");

    set_input(term2, 1, one);
    evaluate_term(term2);
    test_assert(!term2->hasError());
    test_assert(term2->getErrorMessage() == "");
    test_assert(term2->asFloat() == 2.0);
}

void test_eval_as()
{
    test_assert(eval_as<float>("add(1.0,2.0)") == 3);
}

void test_runtime_type_error()
{
    // this test might become invalid when compile-time type checking is added
    Branch branch;
    Term* term = eval_statement(branch, "add('hello', true)");

    evaluate_term(term);

    test_assert(term->hasError());
}

} // namespace runtime_tests

void register_runtime_tests()
{
    REGISTER_TEST_CASE(runtime_tests::test_create_value);
    REGISTER_TEST_CASE(runtime_tests::test_int_value);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::test_find_existing_equivalent);
    REGISTER_TEST_CASE(runtime_tests::var_function_reuse);
    REGISTER_TEST_CASE(runtime_tests::null_input_errors);
    REGISTER_TEST_CASE(runtime_tests::test_eval_as);
    REGISTER_TEST_CASE(runtime_tests::test_runtime_type_error);
}

} // namespace circa
