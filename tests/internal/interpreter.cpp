
#include "framework.h"

#include "branch.h"
#include "evaluation.h"
#include "function.h"
#include "type.h"

using namespace circa;

namespace interpreter {

void test_cast_first_inputs()
{
    // Pass an input of [1] to a branch that expects a compound type.
    // The function will need to cast the [1] to T in order for it to work.

    Branch branch;
    branch.compile("type T { int i }");
    Term* f = branch.compile("def f(T t) -> int { return t.i }");

    EvalContext context;
    push_frame(&context, function_contents(f));

    caValue* in = circa_input((caStack*) &context, 0);
    circa_set_list(in, 1);
    circa_set_int(circa_index(in, 0), 5);

    run_interpreter(&context);

    test_assert(circa_int(circa_output((caStack*) &context, 0)) == 5);
}

}

void interpreter_register_tests()
{
    REGISTER_TEST_CASE(interpreter::test_cast_first_inputs);
}
