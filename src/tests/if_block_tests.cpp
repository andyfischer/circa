// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa_internal.h"
#include "importing_macros.h"

namespace circa {
namespace if_block_tests {

void test_if_joining()
{
    Branch branch;

    // Test that a name defined in one branch is not rebound in outer scope
    branch.eval("if true { apple = 5 }");
    test_assert(!branch.contains("apple"));

    // Test that a name which exists in the outer scope is rebound
    Term* original_banana = create_int(&branch, 10, "banana");
    branch.eval("if true { banana = 15 }");
    test_assert(branch["banana"] != original_banana);

    // Test that if a name is defined in both 'if' and 'else' branches, that it gets
    // defined in the outer scope.
    branch.eval("if true { Cardiff = 5 } else { Cardiff = 11 }");
    test_assert(branch.contains("Cardiff"));

    // Test that the type of the joined name is correct
    branch.compile("if true { a = 4 } else { a = 5 }; a = a");
    test_equals(get_output_type(branch["a"])->name, "int");
}

void test_if_joining_on_bool()
{
    // The following code once had a bug where cond wouldn't work
    // if one of its inputs was missing value.
    Branch branch;
    caValue* s = branch.eval("hey = true");

    test_assert(s->value_data.ptr != NULL);

    branch.eval("if false { hey = false }");

    evaluate_branch(&branch);

    test_assert(branch["hey"]->asBool() == true);
}

void test_if_elif_else()
{
    Branch branch;

    branch.compile("if true { a = 1 } elif true { a = 2 } else { a = 3 } a=a");
    evaluate_branch(&branch);

    test_assert(branch.contains("a"));
    test_equals(branch["a"]->asInt(), 1);

    branch.compile(
        "if false { b = 'apple' } elif false { b = 'orange' } else { b = 'pineapple' } b=b");
    evaluate_branch(&branch);
    test_assert(branch.contains("b"));
    test_assert(branch["b"]->asString() == "pineapple");

    // try one without 'else'
    branch.clear();
    branch.compile("c = 0");
    branch.compile("if false { c = 7 } elif true { c = 8 }; c=c");
    evaluate_branch(&branch);
    test_assert(branch.contains("c"));
    test_assert(branch["c"]->asInt() == 8);

    // try with some more complex conditions
    branch.clear();
    branch.compile("x = 5");
    branch.compile("if x > 6 { compare = 1 } elif x < 6 { compare = -1 } else { compare = 0}");
    branch.compile("compare=compare");
    evaluate_branch(&branch);

    test_assert(branch.contains("compare"));
    test_assert(branch["compare"]->asInt() == -1);
}

void test_dont_always_rebind_inner_names()
{
    Branch branch;
    branch.compile("if false { b = 1 } elif false { c = 1 } elif false { d = 1 } else { e = 1 }");
    evaluate_branch(&branch);
    test_assert(!branch.contains("b"));
    test_assert(!branch.contains("c"));
    test_assert(!branch.contains("d"));
    test_assert(!branch.contains("e"));
}

void test_execution()
{
    Branch branch;

    internal_debug_function::spy_clear();

    // Start off with some simple expressions
    branch.compile("if true { test_spy('Success 1') }");
    branch.compile("if false { test_spy('Fail') }");
    branch.compile("if (1 + 2) > 1 { test_spy('Success 2') }");
    branch.compile("if (1 + 2) < 1 { test_spy('Fail') }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Success 1', 'Success 2']");
    
    // Use 'else'
    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile("if true { test_spy('Success 1') } else { test_spy('Fail') }");
    branch.compile("if false { test_spy('Fail') } else { test_spy('Success 2') }");
    branch.compile("if true { test_spy('Success 3-1') test_spy('Success 3-2') test_spy('Success 3-3') } "
                "else { test_spy('Fail') }");
    branch.compile("if false { test_spy('Fail') test_spy('Fail 2') } "
                "else { test_spy('Success 4-1') test_spy('Success 4-2') test_spy('Success 4-3') }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(),
            "['Success 1', 'Success 2', 'Success 3-1', 'Success 3-2', 'Success 3-3', "
            "'Success 4-1', 'Success 4-2', 'Success 4-3']");

    // Do some nested blocks

    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile("if true { if false { test_spy('Error!') } else { test_spy('Nested 1') } } "
                "else { test_spy('Error!') }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Nested 1']");

    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile("if false { test_spy('Error!') } else { if false { test_spy('Error!') } "
                "else { test_spy('Nested 2') } }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Nested 2']");
    
    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile("if false { test_spy('Error!') }"
                "else { if true { test_spy('Nested 3') } else { test_spy('Error!') } }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Nested 3']");

    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile("if true { if false { test_spy('Error!') } else { test_spy('Nested 4') } } "
                "else { test_spy('Error!') }");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Nested 4']");

    branch.clear();
    internal_debug_function::spy_clear();
    branch.compile(
    "if (false)\n"
    "  test_spy('Error!')\n"
    "else\n"
    "  if (true)\n"
    "    if (false)\n"
    "      test_spy('Error!')\n"
    "    else\n"
    "      if (false)\n"
    "        test_spy('Error!')\n"
    "      else\n"
    "         if (true)\n"
    "           test_spy('Nested 5')\n"
    "         else\n"
    "           test_spy('Error!')\n"
    "           test_spy('Error!')\n"
            );
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Nested 5']");
}

void test_execution_with_elif()
{
    Branch branch;

    branch.compile("x = 5");

    branch.compile("if x > 5 { test_spy('Fail') } "
                "elif x < 5 { test_spy('Fail')} "
                "elif x == 5 { test_spy('Success')} "
                "else { test_spy('Fail') }");

    internal_debug_function::spy_clear();
    evaluate_branch(&branch);
    test_assert(&branch);
    test_equals(internal_debug_function::spy_results(), "['Success']");
}

void test_parse_with_no_line_endings()
{
    Branch branch;

    branch.compile("a = 4");
    branch.compile("if a < 5 { a = 5 }");
    branch.compile("a=a");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_assert(branch["a"]->asInt() == 5);

    branch.compile("if a > 7 { a = 5 } else { a = 3 }");
    branch.compile("a=a");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_assert(branch["a"]->asInt() == 3);

    branch.compile("if a == 2 { a = 1 } elif a == 3 { a = 9 } else { a = 2 }");
    branch.compile("a=a");
    evaluate_branch(&branch);
    test_assert(&branch);
    test_assert(branch["a"]->asInt() == 9);
}

void test_state_simple()
{
    Branch branch;
    Stack context;

    // Simple test, condition never changes
    Term* block = branch.compile("if true { state i = 0; i += 1 }");
    evaluate_branch(&context, &branch);

    caValue *i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(i != NULL);
    test_assert(as_int(i) == 1);
    evaluate_branch(&context, &branch);
    i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context, &branch);
    i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(as_int(i) == 3);

    // Same test with elif
    branch.clear();
    block = branch.compile("if false {} elif true { state i = 0; i += 1 }");
    evaluate_branch(&context, &branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 1);
    evaluate_branch(&context, &branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context, &branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 3);

    // Same test with else
    branch.clear();

    Stack context2;
    block = branch.compile("if false {} else { state i = 0; i += 1 }");
    evaluate_branch(&context2, &branch);
    i = context2.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 1);
    evaluate_branch(&context2, &branch);
    i = context2.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context2, &branch);
    i = context2.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 3);
}

void test_state_is_reset_when_if_fails()
{
    Branch branch;
    Stack context;

    Term* c = branch.compile("c = true");
    branch.compile("if c { state i = 0; i += 1 } else { 'hi' }");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{i: 1}]}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{i: 2}]}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{i: 3}]}");

    set_bool(c, false);

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [null, null]}");

    set_bool(c, true);

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{i: 1}]}");
}

void test_state_is_reset_when_if_fails2()
{
    // Similar to test_state_is_reset_when_if_fails, but this one doesn't
    // have an 'else' block and it uses test_oracle.
    
    internal_debug_function::oracle_clear();

    Branch branch;
    Term* a = branch.compile("a = true");
    
    branch.compile("if a { state s = test_oracle() }");

    internal_debug_function::oracle_send(1);
    internal_debug_function::oracle_send(2);
    internal_debug_function::oracle_send(3);

    Stack context;
    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{s: 1}]}");

    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{s: 1}]}");

    set_bool(a, false);
    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [null, null]}");

    set_bool(a, true);
    evaluate_branch(&context, &branch);
    test_equals(&context.state, "{_if_block: [{s: 2}]}");
}

void register_tests()
{
    REGISTER_TEST_CASE(if_block_tests::test_if_joining);
    REGISTER_TEST_CASE(if_block_tests::test_if_elif_else);
    REGISTER_TEST_CASE(if_block_tests::test_dont_always_rebind_inner_names);
    REGISTER_TEST_CASE(if_block_tests::test_execution);
    REGISTER_TEST_CASE(if_block_tests::test_execution_with_elif);
    REGISTER_TEST_CASE(if_block_tests::test_parse_with_no_line_endings);
    REGISTER_TEST_CASE(if_block_tests::test_state_simple);
    REGISTER_TEST_CASE(if_block_tests::test_state_is_reset_when_if_fails);
    REGISTER_TEST_CASE(if_block_tests::test_state_is_reset_when_if_fails2);
}

} // namespace if_block_tests
} // namespace circa
