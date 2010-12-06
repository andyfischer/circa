// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace subroutine_tests {

void test_return_from_conditional()
{
    Branch branch;
    branch.compile("def my_max(number a, number b) -> number\n"
                "  if (a < b)\n"
                "    return(b)\n"
                "  else\n"
                "    return(a)\n"
                "  end\n"
                "end\n");

    test_assert(branch);

    test_equals(branch.eval("my_max(3,8)")->toFloat(), 8);
    test_equals(branch.eval("my_max(3,3)")->toFloat(), 3);

    test_equals(branch.eval("my_max(11,0)")->toFloat(), 11);
}

void test_recursion()
{
    Branch branch;

    // I think this is the simplest possible recursive function. Evaluate it
    // just to make sure that nothing crashes.
    branch.compile("def recr(bool r) if r recr(false) end end");
    branch.eval("recr(true)");

    // Factorial
    branch.compile("def factorial(int n) -> int\n"
                "  if (n < 2)\n"
                "    return(1)\n"
                "  else\n"
                "    next_i = add_i(n, -1)\n"
                "    return(mult_i(n, factorial(next_i)))\n"
                "  end\n"
                "end");

    TaggedValue* fact_1 = branch.eval("factorial(1)");
    test_assert(branch);
    test_assert(fact_1->asInt() == 1);

    TaggedValue* fact_2 = branch.eval("factorial(2)");
    test_assert(branch);
    test_assert(fact_2->asInt() == 2);

    TaggedValue* fact_3 = branch.eval("factorial(3)");
    test_assert(branch);
    test_assert(fact_3->asInt() == 6);

    TaggedValue* fact_4 = branch.eval("factorial(4)");
    test_assert(branch);
    test_assert(fact_4->asInt() == 24);

    branch.compile("def recr(int n) -> int\n"
                "  if (n == 1)\n"
                "    return(1)\n"
                "  else\n"
                "    return(recr(n - 1) + 1)\n"
                "  end\n"
                "end\n");
    
    test_assert(branch.eval("recr(1)")->asInt() == 1);
    test_assert(branch.eval("recr(2)")->asInt() == 2);
    test_assert(branch.eval("recr(3)")->asInt() == 3);
    test_assert(branch.eval("recr(4)")->asInt() == 4);
}

void test_recursion_with_state()
{
    Branch branch;
    branch.compile("def recr(int i) -> int\n"
                "  state s\n"
                "  if i == 1\n"
                "    return(1)\n"
                "  else\n"
                "    return(recr(i - 1) + 1)\n"
                "  end\n"
                "end\n");

    test_assert(branch);

    dump_branch(branch);

    Term* recr_4 = branch.compile("recr(4)");

    evaluate_branch(branch);
    test_assert(as_int(recr_4) == 4);
}

void subroutine_stateful_term()
{
    EvalContext context;
    Branch branch;
    branch.compile("def mysub()\nstate a = 0.0\na += 1\nend");

    // Make sure that stateful terms work correctly
    Term* call = branch.compile("call = mysub()");
    test_assert(call);
    test_assert(function_t::get_inline_state_type(branch["mysub"]) != VOID_TYPE);
    test_assert(is_function_stateful(branch["mysub"]));

    dump_branch(branch);

    evaluate_branch(&context, branch);

    test_equals(context.state.toString(), "[call: [a: 1.0]]");

    evaluate_branch(&context, branch);

    test_equals(context.state.toString(), "[call: [a: 2.0]]");

    // Make sure that subsequent calls to this subroutine have their own state container.
    branch.compile("another_call = mysub()");
    evaluate_branch(&context, branch);

    test_equals(context.state.toString(), "[call: [a: 2.0], another_call: [a: 1.0]]");
}

void initialize_state_type()
{
    Branch branch;

    Term* a = branch.compile("def a() -> number\nreturn(1 + 1)\nend");
    test_assert(function_t::get_inline_state_type(a) == VOID_TYPE);

    Term* b = branch.compile("def b()\nstate i\nend");
    test_assert(function_t::get_inline_state_type(b) != VOID_TYPE);
}

void shadow_input()
{
    Branch branch;

    // Try having a name that shadows an input. This had a bug at one point
    branch.eval("def f(int i) -> int\ni = 2\nreturn(i)\nend");

    TaggedValue* a = branch.eval("f(1)");

    test_assert(a->asInt() == 2);
}

void specialization_to_output_type()
{
    // If a subroutine is declared with an output type that is more specific
    // than the implicit output type, then make sure that it uses the
    // declared type. This code once had a bug.
    Branch branch;
    Term* a = branch.compile("def a() -> Point\nreturn([1 2])\nend");

    test_assert(function_t::get_output_type(a)->name == "Point");

    // Make sure that the return value is preserved. This had a bug too
    TaggedValue* call = branch.eval("a()");
    test_assert(call->numElements() == 2);
    test_equals(call->getIndex(0)->toFloat(), 1.0);
    test_equals(call->getIndex(1)->toFloat(), 2.0);
}

void stateful_function_with_arguments()
{
    // This code once had a bug
    Branch branch;
    branch.compile("def myfunc(int i) -> int\nstate s\nreturn(i)\nend");
    TaggedValue* call = branch.eval("myfunc(5)");
    test_assert(call->asInt() == 5);
}

void to_source_string()
{
    // This code once had a bug
    Branch branch;
    branch.compile("def f() end");
    Term* c = branch.compile("f()");

    test_equals(get_term_source_text(c), "f()");
}

void bug_with_return()
{
    Branch branch;
    branch.compile("def f(bool b)->int if b return 1 else return 2 end");
    Term* input = branch.compile("b = true");
    Term* f = branch.compile("f(b)");

    // there once was a bug where EvalContext.interruptSubroutine was not reset
    EvalContext context;
    evaluate_branch(&context, branch);
    test_assert(is_int(f));
    test_assert(f->asInt() == 1);

    set_bool(input, false);
    evaluate_branch(&context, branch);
    test_assert(is_int(f));
    test_assert(f->asInt() == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(subroutine_tests::test_return_from_conditional);
    REGISTER_TEST_CASE(subroutine_tests::test_recursion);
    //TEST_DISABLED REGISTER_TEST_CASE(subroutine_tests::test_recursion_with_state);
    //TEST_DISABLED REGISTER_TEST_CASE(subroutine_tests::subroutine_stateful_term);
    REGISTER_TEST_CASE(subroutine_tests::initialize_state_type);
    REGISTER_TEST_CASE(subroutine_tests::shadow_input);
    REGISTER_TEST_CASE(subroutine_tests::specialization_to_output_type);
    REGISTER_TEST_CASE(subroutine_tests::stateful_function_with_arguments);
    REGISTER_TEST_CASE(subroutine_tests::to_source_string);
    REGISTER_TEST_CASE(subroutine_tests::bug_with_return);
}

} // namespace refactoring_tests
} // namespace circa
