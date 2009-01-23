// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace execution_tests {

std::vector<std::string> gSpyResults;

void spy_function(Term * caller)
{
    gSpyResults.push_back(as_string(caller->input(0)));
}

void i_only_throw_errors(Term * caller)
{
    error_occured(caller, "error");
}

void init_test_functions(Branch& branch)
{
    import_function(branch, spy_function, "function spy(string)");
    import_function(branch, i_only_throw_errors,
            "function i-only-throw-errors() -> string");
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
    Term *error = branch.compile("e = i-only-throw-errors()");
    Term *spy_errored = branch.compile("spy(e)");

    test_assert(gSpyResults.size() == 0);

    bool threw = false;
    try {
        evaluate_branch(branch);
    } catch (std::runtime_error const& e) {
        threw = true;
    }

    test_assert(threw);

    test_assert(gSpyResults.size() == 1);
    test_assert(gSpyResults[0] == "1");
    test_assert(!spy_1->hasError());
    test_assert(error->hasError());
    test_assert(spy_errored->needsUpdate);
}

void register_tests()
{
    REGISTER_TEST_CASE(execution_tests::test_simple);
    REGISTER_TEST_CASE(execution_tests::blocked_by_error);
}

} // execution_tests

} // namespace circa
