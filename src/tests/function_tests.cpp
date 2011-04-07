// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* func = branch.eval("def mysub(int) -> string;");

    test_assert(func);
    test_assert(is_subroutine(func));

    test_assert(function_t::get_name(func) == "mysub");

    test_assert(function_t::num_inputs(func) == 1);
    test_assert(function_get_input_type(func, 0) == INT_TYPE);
    test_assert(function_get_output_type(func, 0) == STRING_TYPE);
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
    RefList inputs(a,b);

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
                    MULT_FUNC, RefList(floatInput, intInput))
            ->name, "mult_f");

    Term* add_i = branch.eval("add(1 2)");
    test_equals(add_i->nestedContents[0]->function->name, "add_i");
    Term* add_f = branch.eval("add(1.0 2)");
    test_equals(add_f->nestedContents[0]->function->name, "add_f");

    Term* mult_f = branch.eval("mult(1.0 3)");
    test_equals(mult_f->nestedContents[0]->function->name, "mult_f");
    Term* mult_f_2 = branch.eval("mult(3 1.0)");
    test_equals(mult_f_2->nestedContents[0]->function->name, "mult_f");
}

void overloaded_function_in_script()
{
    Branch branch;
    Term* f1 = branch.compile("def f1(int i);");
    Term* f2 = branch.compile("def f2(int i);");
    
    branch.compile("g = overloaded_function(f1 f2)");
    Term* g_call = branch.compile("g(1)");

    test_assert(g_call->nestedContents[0]->function == f1);

    branch.compile("h = overloaded_function(f2 f1)");
    Term* h_call = branch.compile("h(1)");

    test_assert(h_call->nestedContents[0]->function == f2);
}

void test_is_native_function()
{
    test_assert(is_native_function(KERNEL->get("assert")));

    Branch branch;
    Term* f = branch.eval("def f();");

    test_assert(!is_native_function(f));
}

void test_documentation_string()
{
    Branch branch;
    Term* f = branch.eval("def f() { 'docs' 1 + 2 }");
    test_assert(function_t::get_documentation(f) == "docs");

    Term* f2 = branch.eval("def f2() { a = 'not docs' 1 + 2 }");
    test_assert(function_t::get_documentation(f2) == "");

    Term* f3 = branch.eval("def f2() { print('not docs') 1 + 2 }");
    test_assert(function_t::get_documentation(f3) == "");
}

void test_bug_with_declaring_state_argument()
{
    Branch branch;
    Term* f = branch.eval("def f(state int);");

    test_assert(get_function_attrs(f)->implicitStateType == INT_TYPE);
}

void test_calling_manual_overloaded_function()
{
    Branch branch;
    Term* my_add = branch.compile("my_add = overloaded_function(add_f add_i)");
    test_assert(is_callable(my_add));

    Term* two = branch.compile("2");
    RefList inputs(two, two);

    test_assert(branch);
    test_assert(my_add->function == OVERLOADED_FUNCTION_FUNC);
    test_assert(inputs_statically_fit_function(KERNEL->get("add_f"), inputs));

    Term* sum = branch.compile("my_add(2 2)");
    evaluate_branch(branch);
    test_assert(branch);
    test_equals(sum, "4.0");
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
        Term* f = branch.compile("def f(int a +out, int b +out) -> int");
        install_function(f, f_eval);
        branch.compile("a = 2");
        branch.compile("b = 2");
        branch.compile("f(2,2)");
        branch.compile("c = f(&a, &b)");

        test_assert(branch);
        evaluate_branch(branch);
        test_equals(get_local(branch["a"]), "3");
        test_equals(get_local(branch["b"]), "4");
        test_equals(get_local(branch["c"]), "4");
    }
}

void multiple_output_static_typing()
{
    Branch branch;
    branch.compile("def f(int a +out, string b +out);");
    Term* call = branch.compile("f(1, 'hi')");
    test_assert(get_output_type(call, 1) == INT_TYPE);
    test_assert(get_output_type(call, 2) == STRING_TYPE);
    test_assert(branch);

    branch.compile("c = 2, d = 'hi'");
    branch.compile("f(&c, &d)");
    branch.compile("def needs_int(int);");
    branch.compile("def needs_string(string);");
    branch.compile("needs_int(c)");
    branch.compile("needs_string(d)");
    test_assert(branch);
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
    REGISTER_TEST_CASE(function_tests::test_bug_with_declaring_state_argument);
    REGISTER_TEST_CASE(function_tests::test_calling_manual_overloaded_function);
    REGISTER_TEST_CASE(function_tests::test_bug_where_a_mysterious_copy_term_was_added);
    REGISTER_TEST_CASE(function_tests::test_func_with_multiple_outputs::simple);
    REGISTER_TEST_CASE(function_tests::multiple_output_static_typing);
}

} // namespace function_tests

} // namespace circa
