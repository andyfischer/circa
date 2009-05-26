// Copyright 2008 Andrew Fischer

#include <circa.h>

namespace circa {
namespace subroutine_tests {

void test_return_from_conditional()
{
    Branch branch;
    branch.eval("def my_max(float a, float b) : float\n"
                "  if (a < b)\n"
                "    return b\n"
                "  else\n"
                "    return a\n"
                "  end\n"
                "end\n");

    test_equals(branch.eval("my_max(3,8)")->asFloat(), 8);
    test_equals(branch.eval("my_max(3,3)")->asFloat(), 3);
    test_equals(branch.eval("my_max(11,0)")->asFloat(), 11);
}

void test_recursion()
{
    Branch branch;
    branch.eval("def factorial(int n) : int\n"
                "  if (n < 2)\n"
                "    return 1\n"
                "  else\n"
                "    next_i = add_i(n, -1)\n"
                "    return mult_i(n, factorial(next_i))\n"
                "  end\n"
                "end");

    Term* fact_1 = branch.eval("factorial(1)");
    test_assert(fact_1);
    test_equals(fact_1->asInt(), 1);

    Term* fact_2 = branch.eval("factorial(2)");
    test_assert(fact_2);
    test_equals(fact_2->asInt(), 2);

    Term* fact_3 = branch.eval("factorial(3)");
    test_assert(fact_3);
    test_equals(fact_3->asInt(), 6);

    Term* fact_4 = branch.eval("factorial(4)");
    test_assert(fact_4);
    test_equals(fact_4->asInt(), 24);
}

void subroutine_stateful_term()
{
    Branch branch;
    branch.eval("def mysub()\nstate a :float = 0.0\na += 1\nend");

    // Make sure that stateful terms work correctly
    Term* call = branch.eval("mysub()");
    test_assert(call);
    Term* a_inside_call = get_state_for_subroutine_call(call)->field("a");
    test_equals(as_float(a_inside_call), 1);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 2);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 3);

    // Make sure that subsequent calls to this subroutine don't share
    // the same stateful value.
    Term* another_call = branch.eval("mysub()");
    Term* a_inside_another_call = get_state_for_subroutine_call(another_call)->field("a");
    test_assert(a_inside_call != a_inside_another_call);
    test_equals(as_float(a_inside_another_call), 1);
    evaluate_term(another_call);
    test_equals(as_float(a_inside_another_call), 2);
    test_equals(as_float(a_inside_call), 3);

    // Test accessing the subroutine's state in various ways
    {
        Term* call = branch.compile("mysub()");
        Term* state = get_state_for_subroutine_call(call);
        test_assert(state);
        test_assert(!is_subroutine_state_expanded(state));
        Branch& stateContents = state->asBranch();
        test_assert(stateContents.length() == 0);
        
        expand_subroutines_hidden_state(call, state);
        test_assert(is_subroutine_state_expanded(state));
        test_assert(stateContents.contains("a"));
    }
}

void register_tests()
{
    REGISTER_TEST_CASE(subroutine_tests::test_return_from_conditional);
    REGISTER_TEST_CASE(subroutine_tests::test_recursion);
    REGISTER_TEST_CASE(subroutine_tests::subroutine_stateful_term);
}

} // namespace refactoring_tests

} // namespace circa
