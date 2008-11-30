// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace execution_tests {

std::vector<std::string> gSpyResults;

void spy_function(Term * caller)
{
    gSpyResults.push_back(as_string(caller->inputs[0]));
}

void i_only_throw_errors(Term * caller)
{
    error_occured(caller, "error");
}

void init_test_functions(Branch& branch)
{
    import_c_function(branch, spy_function, "function spy(string)");
    import_c_function(branch, i_only_throw_errors,
            "function i-only-throw-errors() -> string");
}

void test_simple()
{
    Branch branch;
    init_test_functions(branch);

    gSpyResults.clear();

    apply_statement(branch, "spy('1')");
    apply_statement(branch, "spy('2')");
    apply_statement(branch, "spy('3')");

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

    Term *spy_1 = apply_statement(branch, "spy('1')");
    Term *error = apply_statement(branch, "e = i-only-throw-errors()");
    Term *spy_errored = apply_statement(branch, "spy(e)");

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

} // execution_tests

void register_execution_tests()
{
    REGISTER_TEST_CASE(execution_tests::test_simple);
    REGISTER_TEST_CASE(execution_tests::blocked_by_error);
}

} // namespace circa
