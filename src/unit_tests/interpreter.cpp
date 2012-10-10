// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "branch.h"
#include "evaluation.h"
#include "kernel.h"
#include "function.h"
#include "type.h"

namespace interpreter {

void test_cast_first_inputs()
{
    // Pass an input of [1] to a branch that expects a compound type.
    // The function will need to cast the [1] to T in order for it to work.

    Branch branch;
    branch.compile("type T { int i }");
    Term* f = branch.compile("def f(T t) -> int { return t.i }");

    Stack stack;
    push_frame(&stack, function_contents(f));

    caValue* in = circa_input((caStack*) &stack, 0);
    circa_set_list(in, 1);
    circa_set_int(circa_index(in, 0), 5);

    run_interpreter(&stack);

    test_assert(circa_int(circa_output((caStack*) &stack, 0)) == 5);
}

void run_branch_after_additions()
{
    Branch branch;

    // Create a branch and run it.
    branch.compile("a = 1");
    branch.compile("test_spy(a)");
    branch.compile("b = a + 2");
    branch.compile("test_spy(b)");

    test_spy_clear();

    Stack stack;
    push_frame(&stack, &branch);

    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1, 3]");

    // Add some more stuff to the branch, and run it. The Stack was not modified,
    // so it should continue where we stopped.
    branch.compile("c = 4");
    branch.compile("test_spy(c)");
    branch.compile("d = a + b + c");
    branch.compile("test_spy(d)");

    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[4, 8]");
}

} // namespace interpreter

void interpreter_register_tests()
{
    REGISTER_TEST_CASE(interpreter::test_cast_first_inputs);
    REGISTER_TEST_CASE(interpreter::run_branch_after_additions);
}
