// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace for_loop_tests {

std::vector<int> gSpyResults;

void spy_function(Term* caller)
{
    gSpyResults.push_back(as_int(caller->input(0)));
}

void test_simple()
{
    Branch branch;
    import_function(branch, spy_function, "function spy(int)");
    gSpyResults.clear();

    branch.compile("for (int i : range(5))\nspy(i)\nend");

    evaluate_branch(branch);

    test_assert(gSpyResults.size() == 5);
    test_assert(gSpyResults[0] == 0);
    test_assert(gSpyResults[1] == 1);
    test_assert(gSpyResults[2] == 2);
    test_assert(gSpyResults[3] == 3);
    test_assert(gSpyResults[4] == 4);
}

void register_tests()
{
    REGISTER_TEST_CASE(for_loop_tests::test_simple);
}

} // for_loop_tests

} // namespace circa
