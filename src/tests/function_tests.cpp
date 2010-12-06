// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace function_tests {

void create()
{
    Branch branch;

    Term* func = branch.eval("def mysub(int) -> string\nend");

    test_assert(func);
    test_assert(is_subroutine(func));

    test_assert(function_t::get_name(func) == "mysub");

    test_assert(function_t::num_inputs(func) == 1);
    test_assert(function_t::get_input_type(func, 0) == INT_TYPE);
    test_assert(function_t::get_output_type(func) == STRING_TYPE);
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
    Term* f1 = branch.compile("def f1(int i) end");
    Term* f2 = branch.compile("def f2(int i) end");
    
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

    test_assert(function_t::get_inline_state_type(f) == INT_TYPE);
}

void test_call_copied_function()
{
    Branch branch;
    Term* add_copy = branch.compile("add_copy = add");
    test_assert(is_callable(add_copy));
    Term* two = branch.compile("2");
    RefList inputs(two, two);

    //evaluate_without_side_effects(add_copy);

    test_assert(overloaded_function::is_overloaded_function(add_copy));
    test_assert(inputs_statically_fit_function(KERNEL->get("add_i"), inputs));

    test_assert(overloaded_function::statically_specialize_function(add_copy, inputs)
            != UNKNOWN_FUNCTION);

    Term* a = branch.compile("a = add_copy(2 2)");
    test_assert(branch);
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(a->asInt() == 4);

    // Try the test once more, without all of the extra stuff
    branch.compile("add_copy_2 = add");
    Term* b = branch.compile("add_copy_2(3 3)");
    test_assert(branch);
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(b->asInt() == 6);
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
    test_assert(sum->asInt() == 4);
}

void test_bug_where_a_mysterious_copy_term_was_added()
{
    Branch branch;
    branch.compile("def f()->int return(1) end");
    for (BranchIterator it(branch); !it.finished(); ++it)
        test_assert(it->function->name != "copy");
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
    //TEST_DISABLED REGISTER_TEST_CASE(function_tests::test_call_copied_function);
    //TEST_DISABLED REGISTER_TEST_CASE(function_tests::test_calling_manual_overloaded_function);
    REGISTER_TEST_CASE(function_tests::test_bug_where_a_mysterious_copy_term_was_added);
}

} // namespace function_tests

} // namespace circa
