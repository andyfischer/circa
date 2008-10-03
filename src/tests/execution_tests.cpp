// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"
#include "tests/common.h"

namespace circa {
namespace execution_tests {

std::vector<std::string> gSpyResults;

void spy_function(Term * caller)
{
    gSpyResults.push_back(as_string(caller->inputs[0]));
}

void init_spy_function(Branch& branch)
{
    import_c_function(branch, spy_function, "function spy(string)");
}

void test_simple()
{
    Branch branch;
    init_spy_function(branch);

    gSpyResults.clear();

    apply_statement(branch, "spy('1)");
    apply_statement(branch, "spy('2)");
    apply_statement(branch, "spy('3)");

    test_assert(gSpyResults.size() == 0);

    evaluate_branch(branch);

    test_assert(gSpyResults[0] == "1");
    test_assert(gSpyResults[1] == "2");
    test_assert(gSpyResults[2] == "3");
}

} // execution_tests

void register_execution_tests()
{
    REGISTER_TEST_CASE(execution_tests::test_simple);
}

} // namespace circa
