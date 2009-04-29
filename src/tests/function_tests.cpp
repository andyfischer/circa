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

    Term* sub = branch.eval("sub = subroutine_create('my-sub', tuple(int), string)");

    test_assert(!sub->hasError);

    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(identity_equals(as_function(sub).inputTypes[0], INT_TYPE));
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(identity_equals(as_function(sub).outputType, STRING_TYPE));
}

void using_apply()
{
    Branch branch;

    branch.eval("sub = subroutine_create('s', tuple(float), float)");
    branch.eval("function_name_input(@sub, 0, 'x')");
    branch.eval("subroutine_apply(@sub, \"return add(mult(x,2.0),5.0)\")");

    // now run it
    Term* result = branch.eval("result = sub(2.0)");

    test_assert(result);

    test_equals(as_float(result), 9.0);

    // run it again
    Term* result2 = branch.eval("result2 = sub(5.0)");
    test_assert(result2);

    test_assert(as_float(result2) == 15.0);
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
    Term* call = branch.eval("mysub()");
    test_assert(!call->hasError);
    Term* a_inside_call = as_branch(call->state)["a"];
    test_equals(as_float(a_inside_call), 1);
    evaluate_term(call);
    test_equals(as_float(a_inside_call), 2);
    evaluate_term(call);
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
