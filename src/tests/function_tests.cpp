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

    Term* func = branch.eval("def mysub(int):string\nend");

    test_assert(func);
    test_assert(is_subroutine(func));

    test_assert(function_get_name(func) == "mysub");

    test_assert(function_num_inputs(func) == 1);
    test_assert(function_get_input_type(func, 0) == INT_TYPE);
    test_assert(identity_equals(function_get_output_type(func), STRING_TYPE));
}

void test_is_callable()
{
    // normal function
    test_assert(is_callable(COPY_FUNC));

    // overloaded function
    test_assert(is_callable(ADD_FUNC));

    // subroutine
    Branch branch;
    Term* s = branch.eval("def mysub()\nend");
    test_assert(is_callable(s));

    Term* a = branch.eval("1");
    test_assert(!is_callable(a));
}

void test_inputs_fit_function()
{
    Branch branch;
    Term* a = branch.eval("1");
    Term* b = branch.eval("1.0");
    RefList inputs(a,b);

    test_assert(inputs_fit_function(as_branch(ADD_FUNC)["add_f"], inputs));
    test_assert(!inputs_fit_function(as_branch(ADD_FUNC)["add_i"], inputs));
}

void overloaded_function()
{
    Branch branch;

    Term* i = branch.eval("add(1 2)");
    test_equals(i->function->name, "add_i");
    Term* f = branch.eval("add(1.0 2)");
    test_equals(f->function->name, "add_f");
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::test_is_callable);
    REGISTER_TEST_CASE(function_tests::test_inputs_fit_function);
    REGISTER_TEST_CASE(function_tests::overloaded_function);
}

} // namespace function_tests

} // namespace circa
