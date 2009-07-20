// Copyright 2008 Paul Hodge

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

    test_assert(branch);

    test_equals(branch.eval("my_max(3,8)")->asFloat(), 8);
    test_equals(branch.eval("my_max(3,3)")->asFloat(), 3);

    branch.eval("my_max(11,0)");

    test_equals(branch.eval("my_max(11,0)")->asFloat(), 11);
}

void test_recursion()
{
    Branch branch;
    branch.eval("def factorial(int n) : int\n"
                "  state i\n" // TODO: remove this
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
    test_assert(function_t::get_hidden_state_type(branch["mysub"]) != VOID_TYPE);
    test_assert(is_function_stateful(branch["mysub"]));
    test_assert(get_hidden_state_for_call(call) != NULL);
    Term* a_inside_call = get_hidden_state_for_call(call)->field("a");
    test_equals(as_float(a_inside_call), 1);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 2);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 3);

    // Make sure that subsequent calls to this subroutine don't share
    // the same stateful value.
    Term* another_call = branch.eval("mysub()");
    Term* a_inside_another_call = get_hidden_state_for_call(another_call)->field("a");
    test_assert(a_inside_call != a_inside_another_call);
    test_equals(as_float(a_inside_another_call), 1);
    evaluate_term(another_call);
    test_equals(as_float(a_inside_another_call), 2);
    test_equals(as_float(a_inside_call), 3);

    // Test accessing the subroutine's state in various ways
    {
        Term* call = branch.compile("mysub()");
        Term* state = get_hidden_state_for_call(call);
        test_assert(state);
        test_assert(!is_subroutine_state_expanded(state));
        Branch& stateContents = state->asBranch();
        test_assert(stateContents.length() == 0);
        
        expand_subroutines_hidden_state(call, state);
        test_assert(is_subroutine_state_expanded(state));
        test_assert(stateContents.contains("a"));
    }
}

void initialize_state_type()
{
    Branch branch;

    Term* a = branch.eval("def a():float\nreturn 1 + 1\nend");
    test_assert(function_t::get_hidden_state_type(a) == VOID_TYPE);

    Term* b = branch.eval("def b()\nstate i\nend");
    test_assert(function_t::get_hidden_state_type(b) == BRANCH_TYPE);
}

void shadow_input()
{
    Branch branch;

    // Try having a name that shadows an input. This had a bug at one point
    branch.eval("def f(int i):int\ni = 2\nreturn i\nend");

    Term* a = branch.eval("f(1)");

    test_assert(a->asInt() == 2);
}

void specialization_to_output_type()
{
    // If a subroutine is declared with an output type that is more specific
    // than the implicit output type, then make sure that it uses the
    // declared type. This code once had a bug.
    Branch branch;
    Term* point = branch.eval("type Point { float x, float y }");
    Term* a = branch.eval("def a() : Point\nreturn [1 2]\nend");

    test_assert(function_t::get_output_type(a) == point);

    // Make sure that the return value is preserved. This had a bug too
    Term* call = branch.eval("a()");
    test_assert(as_branch(call).length() == 2);
    test_equals(call->field(0)->toFloat(), 1.0);
    test_equals(call->field(1)->toFloat(), 2.0);
}

void stateful_function_with_arguments()
{
    // This code once had a bug
    Branch branch;
    branch.eval("def myfunc(int i) : int\nstate s\nreturn i\nend");
    Term* call = branch.eval("myfunc(5)");
    test_assert(call->asInt() == 5);
}

void register_tests()
{
    REGISTER_TEST_CASE(subroutine_tests::test_return_from_conditional);
    //FIXME REGISTER_TEST_CASE(subroutine_tests::test_recursion);
    REGISTER_TEST_CASE(subroutine_tests::subroutine_stateful_term);
    REGISTER_TEST_CASE(subroutine_tests::initialize_state_type);
    REGISTER_TEST_CASE(subroutine_tests::shadow_input);
    REGISTER_TEST_CASE(subroutine_tests::specialization_to_output_type);
    REGISTER_TEST_CASE(subroutine_tests::stateful_function_with_arguments);
}

} // namespace refactoring_tests

} // namespace circa
