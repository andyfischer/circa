// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace runtime_tests {

std::vector<std::string> gSpyResults;

void spy_function(Term* caller)
{
    gSpyResults.push_back(as_string(caller->input(0)));
}

void i_only_throw_errors(Term* caller)
{
    error_occured(caller, "error");
}

void init_test_functions(Branch& branch)
{
    import_function(branch, spy_function, "spy(string)");
    import_function(branch, i_only_throw_errors, "i_only_throw_errors() : string");
}

void test_simple()
{
    Branch branch;
    init_test_functions(branch);

    gSpyResults.clear();

    branch.compile("spy('1')");
    branch.compile("spy('2')");
    branch.compile("spy('3')");

    test_assert(gSpyResults.size() == 0);

    evaluate_branch(branch);

    test_assert(gSpyResults[0] == "1");
    test_assert(gSpyResults[1] == "2");
    test_assert(gSpyResults[2] == "3");
}

void blocked_by_error()
{
    Branch branch;
    init_test_functions(branch);

    gSpyResults.clear();

    Term *spy_1 = branch.compile("spy('1')");
    Term *error = branch.compile("e = i_only_throw_errors()");
    Term *spy_errored = branch.compile("spy(e)");

    test_assert(gSpyResults.size() == 0);

    bool threw = false;
    try {
        evaluate_branch(branch);
    } catch (std::runtime_error const&) {
        threw = true;
    }

    test_assert(threw);

    test_assert(gSpyResults.size() == 1);
    test_assert(gSpyResults[0] == "1");
    test_assert(!spy_1->hasError);
    test_assert(error->hasError);
}


void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(TYPE_TYPE->type == TYPE_TYPE);

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(FUNCTION_TYPE->type == TYPE_TYPE);
}

void null_input_errors()
{
    Branch branch;

    Term* one = float_value(&branch, 1.0);

    Term* term1 = apply(&branch, ADD_FUNC, RefList(NULL, one));
    evaluate_term(term1);
    test_assert(term1->hasError);
    test_assert(term1->getErrorMessage() == "Input 0 is NULL");

    term1->function = NULL;
    evaluate_term(term1);
    test_assert(term1->hasError);
    test_assert(term1->getErrorMessage() == "Function is NULL");

    Term* term2 = apply(&branch, ADD_FUNC, RefList(one, NULL));
    evaluate_term(term2);
    test_assert(term2->hasError);
    test_assert(term2->getErrorMessage() == "Input 1 is NULL");

    set_input(term2, 1, one);
    evaluate_term(term2);
    test_assert(!term2->hasError);
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
    Term* term = branch.eval("add('hello', true)");
    evaluate_term(term);
    test_assert(term->hasError);

    // try wrong # of arguments
    term = branch.eval("div(1)");
    evaluate_term(term);
    test_assert(term->hasError);
}

void test_resize_list()
{
    Branch list;

    int_value(&list, 1, "a");
    int_value(&list, 2, "b");

    resize_list(list, 4, INT_TYPE);

    test_assert(list.length() == 4);
    test_assert(list[0]->asInt() == 1);
    test_assert(list[0]->name == "a");
    test_assert(list[1]->asInt() == 2);
    test_assert(list[1]->name == "b");
    test_assert(list[2]->asInt() == 0);
    test_assert(list[3]->asInt() == 0);

    resize_list(list, 1, INT_TYPE);
    test_assert(list.length() == 1);
    test_assert(list[0]->asInt() == 1);
    test_assert(list[0]->name == "a");

    resize_list(list, 3, STRING_TYPE);
    test_assert(list.length() == 3);
    test_assert(list[1]->type == STRING_TYPE);
    test_assert(list[2]->type == STRING_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_tests::test_simple);
    REGISTER_TEST_CASE(runtime_tests::blocked_by_error);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::null_input_errors);
    REGISTER_TEST_CASE(runtime_tests::test_eval_as);
    REGISTER_TEST_CASE(runtime_tests::test_runtime_type_error);
    REGISTER_TEST_CASE(runtime_tests::test_resize_list);
}

} // namespace runtime_tests

} // namespace circa
