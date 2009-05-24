// Copyright 2008 Paul Hodge

#include <circa.h>

namespace circa {
namespace subroutine_tests {

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
    REGISTER_TEST_CASE(subroutine_tests::subroutine_stateful_term);
}

} // namespace refactoring_tests

} // namespace circa
