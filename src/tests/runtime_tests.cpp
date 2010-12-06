// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace runtime_tests {

std::vector<std::string> gSpyResults;

CA_FUNCTION(spy_function)
{
    gSpyResults.push_back(as_string(INPUT(0)));
    set_bool(OUTPUT, true);
}

CA_FUNCTION(i_only_throw_errors)
{
    error_occurred(CONTEXT, CALLER, "i only throw errors");
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

    branch.compile("spy('1')");
    Term *error = branch.compile("e = i_only_throw_errors()");
    Term *spy_blocked = branch.compile("spy(e)");

    test_assert(gSpyResults.size() == 0);

    EvalContext result = evaluate_branch(branch);
    test_assert(result.errorOccurred);
    test_assert(result.errorTerm == error);
    test_assert(gSpyResults.size() == 1);
    test_assert(gSpyResults[0] == "1");

    test_assert(as_bool(spy_blocked) == false);
}

void test_misc()
{
    test_assert(is_type(TYPE_TYPE));
    test_assert(TYPE_TYPE->type == TYPE_TYPE);

    test_assert(is_type(FUNCTION_TYPE));
    test_assert(FUNCTION_TYPE->type == TYPE_TYPE);
}

void function_that_ignores_errors()
{
#if 0
    Branch branch;
    init_test_functions(branch);

    branch.eval("a = i_only_throw_errors()");
    Term* mirror = branch.eval("branch_ref(a)");

    test_assert(function_t::get_input_placeholder(mirror->function, 0)
            ->boolPropOptional("ignore_error", false));

    test_assert(!has_static_error(mirror));
#endif
}

void register_tests()
{
    REGISTER_TEST_CASE(runtime_tests::test_simple);
    //TEST_DISABLED REGISTER_TEST_CASE(runtime_tests::blocked_by_error);
    REGISTER_TEST_CASE(runtime_tests::test_misc);
    REGISTER_TEST_CASE(runtime_tests::function_that_ignores_errors);
}

} // namespace runtime_tests

} // namespace circa
