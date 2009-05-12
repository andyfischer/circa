// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "testing.h"
#include "introspection.h"
#include "runtime.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* sub = branch.eval("def mysub(int) : string\nend");

    test_assert(sub);
    test_assert(is_subroutine(sub));

    Function& func = get_subroutines_function_def(sub);

    test_assert(func.name == "mysub");

    // the inputs are [Branch int] because we assume that every
    // subroutine has hidden state
    test_assert(identity_equals(func.inputTypes[1], INT_TYPE));
    test_assert(func.inputTypes.count() == 2);
    test_assert(identity_equals(func.outputType, STRING_TYPE));
}

void subroutine_stateful_term()
{
    Branch branch;
    branch.eval("def mysub()\nstate a :float = 0.0\na += 1\nend");

    // Make sure that stateful terms work correctly
    Term* call = branch.eval("mysub()");
    test_assert(call);
    Term* a_inside_call = get_state_for_subroutine_call(call)["a"];
    test_equals(as_float(a_inside_call), 1);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 2);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 3);

    // Make sure that subsequent calls to this subroutine don't share
    // the same stateful value.
    Term* another_call = branch.eval("mysub()");
    Term* a_inside_another_call = get_state_for_subroutine_call(another_call)["a"];
    test_assert(a_inside_call != a_inside_another_call);
    test_equals(as_float(a_inside_another_call), 1);
    evaluate_term(another_call);
    test_equals(as_float(a_inside_another_call), 2);
    test_equals(as_float(a_inside_call), 3);
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::subroutine_stateful_term);
}

} // namespace function_tests

} // namespace circa
