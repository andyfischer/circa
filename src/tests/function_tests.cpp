// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* func = branch.eval("def mysub(int)::string\nend");

    test_assert(func);
    test_assert(is_subroutine(func));

    test_assert(function_t::get_name(func) == "mysub");

    test_assert(function_t::num_inputs(func) == 1);
    test_assert(function_t::get_input_type(func, 0) == INT_TYPE);
    test_assert(identity_equals(function_t::get_output_type(func), STRING_TYPE));
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

    test_assert(inputs_fit_function(KERNEL->get("add_f"), inputs));
    test_assert(!inputs_fit_function(KERNEL->get("add_i"), inputs));
}

void overloaded_function()
{
    Branch branch;

    Term* i = branch.eval("add(1 2)");
    test_equals(i->function->name, "add_i");
    Term* f = branch.eval("add(1.0 2)");
    test_equals(f->function->name, "add_f");

    Term* myfunc_f = branch.eval("def myfunc +overload (number)::number return 2.0 end");
    Term* myfunc_i = branch.eval("def myfunc +overload (int)::int return 1 end");

    test_assert(branch);

    Term* int_input = branch.eval("1");
    Term* number_input = branch.eval("1.0");

    test_assert(specialize_function(myfunc_i, RefList(int_input)) == myfunc_i);
    test_assert(specialize_function(myfunc_i, RefList(number_input)) == myfunc_f);

    branch.eval("myfunc(1) myfunc(1.0)");

    test_assert(branch.eval("myfunc(1) == 1"));
    test_assert(branch.eval("myfunc(1.0) == 2.0"));
}

void test_is_native_function()
{
    test_assert(is_native_function(KERNEL->get("assert")));

    Branch branch;
    Term* f = branch.eval("def f() end");

    test_assert(!is_native_function(f));
}

void test_documentation_string()
{
    Branch branch;
    Term* f = branch.eval("def f() 'docs' 1 + 2 end");
    test_assert(function_t::get_documentation(f) == "docs");

    Term* f2 = branch.eval("def f2() a = 'not docs' 1 + 2 end");
    test_assert(function_t::get_documentation(f2) == "");

    Term* f3 = branch.eval("def f2() print('not docs') 1 + 2 end");
    test_assert(function_t::get_documentation(f3) == "");
}

void test_bug_with_declaring_state_argument()
{
    Branch branch;
    Term* f = branch.eval("def f(state int) end");

    test_assert(function_t::get_hidden_state_type(f) == INT_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::test_is_callable);
    REGISTER_TEST_CASE(function_tests::test_inputs_fit_function);
    REGISTER_TEST_CASE(function_tests::overloaded_function);
    REGISTER_TEST_CASE(function_tests::test_is_native_function);
    REGISTER_TEST_CASE(function_tests::test_documentation_string);
    REGISTER_TEST_CASE(function_tests::test_bug_with_declaring_state_argument);
}

} // namespace function_tests

} // namespace circa
