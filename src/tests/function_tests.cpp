// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* func = branch.eval("def mysub(int) -> string;");
    Function* attrs = as_function(func);

    test_assert(func);
    test_assert(is_subroutine(func));

    test_assert(attrs->name == "mysub");

    test_assert(function_num_inputs(attrs) == 1);
    test_equals(function_get_input_type(attrs, 0)->name, "int");
    test_equals(function_get_output_type(attrs, 0)->name, "string");
}

void test_is_callable()
{
    // normal function
    test_assert(is_callable(COPY_FUNC));

    // overloaded function
    test_assert(is_callable(ADD_FUNC));

    // subroutine
    Branch branch;
    Term* s = branch.eval("def mysub();");
    test_assert(is_callable(s));

    Term* a = branch.eval("1");
    test_assert(!is_callable(a));
}

void test_inputs_fit_function()
{
    Branch branch;
    Term* a = branch.eval("1");
    Term* b = branch.eval("1.0");
    TermList inputs(a,b);

    test_assert(inputs_statically_fit_function(KERNEL->get("add_f"), inputs));
    test_assert(!inputs_statically_fit_function(KERNEL->get("add_i"), inputs));
}

void overloaded_function()
{
    Branch branch;

    Term* floatInput = branch.eval("5.2");
    Term* intInput = branch.eval("11");

    test_assert(overloaded_function::is_overloaded_function(MULT_FUNC));

    // Test statically_specialize_function
    test_equals(overloaded_function::statically_specialize_function(
                    MULT_FUNC, TermList(floatInput, intInput))
            ->name, "mult_f");

    Term* add_i = branch.eval("add(1 2)");
    test_equals(add_i->contents(0)->function->name, "add_i");
    Term* add_f = branch.eval("add(1.0 2)");
    test_equals(add_f->contents(0)->function->name, "add_f");

    Term* mult_f = branch.eval("mult(1.0 3)");
    test_equals(mult_f->contents(0)->function->name, "mult_f");
    Term* mult_f_2 = branch.eval("mult(3 1.0)");
    test_equals(mult_f_2->contents(0)->function->name, "mult_f");
}

void overloaded_function_in_script()
{
    Branch branch;
    Term* f1 = branch.compile("def f1(int i);");
    Term* f2 = branch.compile("def f2(int i);");
    
    branch.compile("g = overloaded_function(f1 f2)");
    Term* g_call = branch.compile("g(1)");

    test_assert(g_call->contents(0)->function == f1);

    branch.compile("h = overloaded_function(f2 f1)");
    Term* h_call = branch.compile("h(1)");

    test_assert(h_call->contents(0)->function == f2);
}

void test_is_native_function()
{
    test_assert(is_native_function(as_function(KERNEL->get("assert"))));

    Branch branch;
    Term* f = branch.eval("def f();");

    test_assert(!is_native_function(as_function(f)));
}

void test_documentation_string()
{
    Branch branch;
    Term* f = branch.eval("def f() { 'docs' 1 + 2 }");
    test_assert(function_get_documentation_string(as_function(f)) == "docs");

    Term* f2 = branch.eval("def f2() { a = 'not docs' 1 + 2 }");
    test_assert(function_get_documentation_string(as_function(f2)) == "");

    Term* f3 = branch.eval("def f2() { print('not docs') 1 + 2 }");
    test_assert(function_get_documentation_string(as_function(f3)) == "");
}

void test_bug_where_a_mysterious_copy_term_was_added()
{
    Branch branch;
    branch.compile("def f()->int { return(1) }");
    for (BranchIterator it(&branch); !it.finished(); ++it)
        test_assert(it->function->name != "copy");
}

namespace test_func_with_multiple_outputs {
    CA_FUNCTION(f_eval) {
        set_int(EXTRA_OUTPUT(0), INT_INPUT(0) + 1);
        set_int(EXTRA_OUTPUT(1), INT_INPUT(1) + 2);
        set_int(OUTPUT, INT_INPUT(0) + INT_INPUT(1));
    }

    void simple()
    {
        Branch branch;
        Term* f = branch.compile("def f(int a :out, int b :out) -> int");
        install_function(f, f_eval);
        branch.compile("a = 2");
        branch.compile("b = 2");
        branch.compile("f(2,2)");
        branch.compile("c = f(&a, &b)");

        test_assert(&branch);
        evaluate_branch(&branch);
        test_equals(get_local(branch["a"]), "3");
        test_equals(get_local(branch["b"]), "4");
        test_equals(get_local(branch["c"]), "4");
    }
}

void multiple_output_static_typing()
{
    Branch branch;
    branch.compile("def f(int a :out, string b :out);");
    Term* call = branch.compile("f(1, 'hi')");
    test_equals(get_output_type(call, 1)->name, "int");
    test_equals(get_output_type(call, 2)->name, "string");
    test_assert(&branch);

    branch.compile("c = 2, d = 'hi'");
    branch.compile("f(&c, &d)");
    branch.compile("def needs_int(int);");
    branch.compile("def needs_string(string);");
    branch.compile("needs_int(c)");
    branch.compile("needs_string(d)");
    test_assert(&branch);
}

void test_num_explicit_inputs()
{
    Branch branch;

    Function* f = as_function(branch.compile("def f(int i)"));
    test_equals(function_num_inputs(f), 1);
    test_equals(function_num_explicit_inputs(f), 1);

    function_insert_state_input(f);
    test_equals(function_num_inputs(f), 2);
    test_equals(function_num_explicit_inputs(f), 1);

    Function* g = as_function(branch.compile("def g(state any s, int i)"));
    test_equals(function_num_inputs(g), 2);
    test_equals(function_num_explicit_inputs(g), 1);
}

void register_tests()
{
    REGISTER_TEST_CASE(function_tests::create);
    REGISTER_TEST_CASE(function_tests::test_is_callable);
    REGISTER_TEST_CASE(function_tests::test_inputs_fit_function);
    REGISTER_TEST_CASE(function_tests::overloaded_function);
    REGISTER_TEST_CASE(function_tests::overloaded_function_in_script);
    REGISTER_TEST_CASE(function_tests::test_is_native_function);
    REGISTER_TEST_CASE(function_tests::test_documentation_string);
    REGISTER_TEST_CASE(function_tests::test_bug_where_a_mysterious_copy_term_was_added);
    //TEST_DISABLED REGISTER_TEST_CASE(function_tests::test_func_with_multiple_outputs::simple);
    //TEST_DISABLED REGISTER_TEST_CASE(function_tests::multiple_output_static_typing);
    REGISTER_TEST_CASE(function_tests::test_num_explicit_inputs);
}

} // namespace function_tests

} // namespace circa
