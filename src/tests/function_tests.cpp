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

    Term* sub = eval_statement(branch, "sub = subroutine-create('my-sub', tuple(int), string)");

    test_assert(!sub->hasError());

    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(as_function(sub).inputTypes[0] == INT_TYPE);
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(as_function(sub).outputType == STRING_TYPE);

    // name input
    sub = eval_statement(branch, "function-name-input(@sub, 0, 'apple')");

    test_assert(is_function(sub));
    test_assert(as_function(sub).name == "my-sub");
    test_assert(as_function(sub).inputTypes[0] == INT_TYPE);
    test_assert(as_function(sub).inputTypes.count() == 1);
    test_assert(as_function(sub).outputType == STRING_TYPE);

    Term *input_name = eval_statement(branch, "in = function-get-input-name(sub, 0)");
    test_assert(input_name->asString() == "apple");
}

void using_apply()
{
    Branch branch;

    eval_statement(branch, "sub = subroutine-create('s', tuple(float), float)");
    eval_statement(branch, "function-name-input(@sub, 0, 'x')");
    eval_statement(branch, "subroutine-apply(@sub, \"return add(mult(x,2.0),5.0)\")");

    // now run it
    Term* result = eval_statement(branch, "result = sub(2.0)");

    test_assert(result);

    test_equals(as_float(result), 9.0);

    // run it again
    Term* result2 = eval_statement(branch, "result2 = sub(5.0)");
    test_assert(result2);

    test_assert(as_float(result2) == 15.0);
}

void external_pointers()
{
    Branch branch;

    Term* function = create_value(&branch, FUNCTION_TYPE);

    as_function(function).inputTypes = ReferenceList(INT_TYPE, INT_TYPE);
    as_function(function).outputType = STRING_TYPE;

    test_equals(list_all_pointers(function), ReferenceList(
            function->function,
            FUNCTION_TYPE,
            INT_TYPE,
            STRING_TYPE));

    function = eval_statement(branch,
            "subroutine-create(\"mysub\",tuple(float,float),bool)");

    test_equals(list_all_pointers(function), ReferenceList(
            function->input(0),
            function->input(1), // a list of (float,float)
            BOOL_TYPE,
            KERNEL->findNamed("subroutine-create"),
            FUNCTION_TYPE,
            FLOAT_TYPE,
            BRANCH_TYPE,
            get_value_function(*KERNEL, FLOAT_TYPE)
            ));
}

void subroutine_binding_input_names()
{
    Branch branch;

    Term* mysub = eval_statement(branch,
            "mysub = subroutine-create('mysub', tuple(int), void)");
    test_assert(mysub != NULL);

    mysub = eval_statement(branch, "function-name-input(@mysub, 0, 'a')");

    test_assert(as_function(mysub).subroutineBranch.findNamed("a") != NULL);
}

} // namespace function_tests

void register_function_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::using_apply);
    REGISTER_TEST_CASE(function_tests::external_pointers);
    REGISTER_TEST_CASE(function_tests::subroutine_binding_input_names);
}

} // namespace circa
