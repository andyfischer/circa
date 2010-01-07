// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace runtime_tests {

std::vector<std::string> gSpyResults;

void spy_function(Term* caller)
{
    gSpyResults.push_back(as_string(caller->input(0)));
    as_bool(caller) = true;
}

void i_only_throw_errors(Term* caller)
{
    error_occurred(caller, "i only throw errors");
}

void init_test_functions(Branch& branch)
{
    import_function(branch, spy_function, "spy(string) -> bool");
    import_function(branch, i_only_throw_errors, "i_only_throw_errors() -> string");
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
    Term *spy_blocked = branch.compile("spy(e)");

    test_assert(gSpyResults.size() == 0);

    Term errorListener;
    evaluate_branch(branch, &errorListener);
    test_assert(errorListener.hasError());
    test_assert(gSpyResults.size() == 1);
    test_assert(gSpyResults[0] == "1");
    test_assert(!spy_1->hasError());
    test_assert(error->hasError());

    test_assert(as_bool(spy_blocked) == false);
}

void error_message()
{
    Branch branch;
    init_test_functions(branch);

    branch.eval("def hey() i_only_throw_errors() end");

    test_assert(!has_runtime_error(branch));

    branch.eval("hey()");

    test_assert(has_runtime_error(branch));

    std::stringstream out;
    print_runtime_error_formatted(branch, out);

    if (out.str().find("!!! no error found") != std::string::npos) {
        std::cout << "In runtime_tests::error_message:" << std::endl;
        std::cout << out.str();
        declare_current_test_failed();
    }
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(TYPE_TYPE->type == TYPE_TYPE);

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(FUNCTION_TYPE->type == TYPE_TYPE);
}

void test_resize_list()
{
    Branch list;

    create_int(list, 1, "a");
    create_int(list, 2, "b");

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

void function_that_ignores_errors()
{
    Branch branch;
    Term* a = branch.eval("a = {}");
    Term* mirror = branch.eval("branch_mirror(a)");

    error_occurred(a, "test error");

    test_assert(function_t::get_input_placeholder(mirror->function, 0)
            ->boolPropOptional("ignore_error", false));

    test_assert(!has_static_error(mirror));
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_tests::test_simple);
    REGISTER_TEST_CASE(runtime_tests::blocked_by_error);
    REGISTER_TEST_CASE(runtime_tests::error_message);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::test_resize_list);
    REGISTER_TEST_CASE(runtime_tests::function_that_ignores_errors);
}

} // namespace runtime_tests

} // namespace circa
