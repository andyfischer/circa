// Copyright 2008 Andrew Fischer

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
    import_function(branch, spy_function, "spy(int)");
    gSpyResults.clear();

    branch.compile("for i in range(5)\nspy(i)\nend");

    evaluate_branch(branch);

    test_assert(gSpyResults.size() == 5);
    test_assert(gSpyResults[0] == 0);
    test_assert(gSpyResults[1] == 1);
    test_assert(gSpyResults[2] == 2);
    test_assert(gSpyResults[3] == 3);
    test_assert(gSpyResults[4] == 4);
}

void test_subroutine_call()
{
    Branch branch;

    Term* sub = branch.compile("def myfunc()\nfor i in range(5)\ni\nend\nend");

    Term* forTerm = as_function(sub).subroutineBranch[2];

    test_assert(forTerm->function == FOR_FUNC);

    test_equals("i", get_for_loop_iterator(forTerm)->name);

    Term* call = branch.compile("myfunc()");
    initialize_subroutine_call(call);
    Term* forTermInsideCall = as_branch(call->state)[2];
    test_assert(forTermInsideCall->function == FOR_FUNC);
    test_equals("i", get_for_loop_iterator(forTermInsideCall)->name);
}

void test_state()
{
    Branch branch;

    // Make sure that each iteration of the loop has a different copy of stateful values
    Term* loop = branch.compile("for i in [1 2]\nstate s = unique_id()\nend");

    evaluate_term(loop);

    Term* s_iter0 = get_for_loop_state(loop, 0)["s"];
    Term* s_iter1 = get_for_loop_state(loop, 1)["s"];
    test_assert(s_iter0 != NULL);
    test_assert(s_iter1 != NULL);
    test_assert(as_int(s_iter0) != as_int(s_iter1));
}

void register_tests()
{
    REGISTER_TEST_CASE(for_loop_tests::test_simple);
    REGISTER_TEST_CASE(for_loop_tests::test_subroutine_call);
    REGISTER_TEST_CASE(for_loop_tests::test_state);
}

} // for_loop_tests

} // namespace circa
