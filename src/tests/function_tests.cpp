// Copyright 2008 Paul Hodge

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

    Term* sub = branch.eval("sub = subroutine_create('my-sub', tuple(int), string)");

    test_assert(!sub->hasError);

    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(identity_equals(as_function(sub).inputTypes[0], INT_TYPE));
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(identity_equals(as_function(sub).outputType, STRING_TYPE));
}

void subroutine_binding_input_names()
{
    Branch branch;

    Term* mysub = branch.eval("mysub = subroutine_create('mysub', tuple(int), void)");
    test_assert(mysub != NULL);

    mysub = branch.eval("function_name_input(@mysub, 0, 'a')");

    test_assert(find_named(&as_function(mysub).subroutineBranch,"a") != NULL);
}

void subroutine_stateful_term()
{
    Branch branch;
    branch.eval("def mysub()\nstate a :float = 0\na += 1\nend");

    // Make sure that stateful terms work correctly
    Term* call = branch.eval("mysub()");
    test_assert(!call->hasError);
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
    REGISTER_TEST_CASE(function_tests::subroutine_binding_input_names);
    REGISTER_TEST_CASE(function_tests::subroutine_stateful_term);
}

} // namespace function_tests

} // namespace circa
