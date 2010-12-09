// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "importing_macros.h"

namespace circa {
namespace if_block_tests {

void test_if_joining()
{
    Branch branch;

    // Test that a name defined in one branch is not rebound in outer scope
    branch.eval("if true\napple = 5\nend");
    test_assert(!branch.contains("apple"));

    // Test that a name which exists in the outer scope is rebound
    Term* original_banana = create_int(branch, 10, "banana");
    branch.eval("if true\nbanana = 15\nend");
    test_assert(branch["banana"] != original_banana);

    // Test that if a name is defined in both 'if' and 'else branches, that it gets defined 
    // in the outer scope.
    branch.eval("if true\nCardiff = 5\nelse\nCardiff = 11\nend");
    test_assert(branch.contains("Cardiff"));
}

void test_if_joining_on_bool()
{
    // The following code once had a bug where cond wouldn't work
    // if one of its inputs was missing value.
    Branch branch;
    TaggedValue* s = branch.eval("hey = true");

    test_assert(s->value_data.ptr != NULL);

    branch.eval("if false\nhey = false\nend");

    evaluate_branch(branch);

    test_assert(branch["hey"]->asBool() == true);
}

void test_if_elif_else()
{
    Branch branch;

    branch.compile("if true; a = 1; elif true; a = 2; else; a = 3; end; a=a");
    evaluate_branch(branch);

    test_assert(branch.contains("a"));
    test_assert(branch["a"]->asInt() == 1);

    branch.compile(
        "if false; b = 'apple'; elif false; b = 'orange'; else; b = 'pineapple'; end; b=b");
    evaluate_branch(branch);
    test_assert(branch.contains("b"));
    test_assert(branch["b"]->asString() == "pineapple");

    // try one without 'else'
    branch.clear();
    branch.compile("c = 0");
    branch.compile("if false; c = 7; elif true; c = 8; end; c=c");
    evaluate_branch(branch);
    test_assert(branch.contains("c"));
    test_assert(branch["c"]->asInt() == 8);

    // try with some more complex conditions
    branch.clear();
    branch.compile("x = 5");
    branch.compile("if x > 6; compare = 1; elif x < 6; compare = -1; else; compare = 0; end");
    branch.compile("compare=compare");
    evaluate_branch(branch);

    test_assert(branch.contains("compare"));
    test_assert(branch["compare"]->asInt() == -1);
}

void test_dont_always_rebind_inner_names()
{
    Branch branch;
    branch.compile("if false; b = 1; elif false; c = 1; elif false; d = 1; else; e = 1; end");
    evaluate_branch(branch);
    test_assert(!branch.contains("b"));
    test_assert(!branch.contains("c"));
    test_assert(!branch.contains("d"));
    test_assert(!branch.contains("e"));
}

std::vector<std::string> gSpyResults;

CA_FUNCTION(spy_function)
{
    gSpyResults.push_back(as_string(INPUT(0)));
}

void test_execution()
{
    Branch branch;
    import_function(branch, spy_function, "spy(string)");
    gSpyResults.clear();

    // Start off with some simple expressions
    branch.compile("if true\n spy('Success 1')\nend");
    branch.compile("if false\n spy('Fail')\nend");
    branch.compile("if (1 + 2) > 1\n spy('Success 2')\nend");
    branch.compile("if (1 + 2) < 1\n spy('Fail')\nend");
    branch.compile("if true; spy('Success 3'); end");
    branch.compile("if false; spy('Fail'); end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 3);
    test_equals(gSpyResults[0], "Success 1");
    test_equals(gSpyResults[1], "Success 2");
    test_equals(gSpyResults[2], "Success 3");
    
    // Use 'else'
    gSpyResults.clear();
    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile("if true; spy('Success 1'); else; spy('Fail'); end");
    branch.compile("if false; spy('Fail'); else; spy('Success 2'); end");
    branch.compile("if true; spy('Success 3-1')\n spy('Success 3-2')\n spy('Success 3-3')\n"
                "else; spy('Fail'); end");
    branch.compile("if false; spy('Fail')\n spy('Fail 2')\n"
                "else; spy('Success 4-1')\n spy('Success 4-2')\n spy('Success 4-3')\n end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 8);
    test_equals(gSpyResults[0], "Success 1");
    test_equals(gSpyResults[1], "Success 2");
    test_equals(gSpyResults[2], "Success 3-1");
    test_equals(gSpyResults[3], "Success 3-2");
    test_equals(gSpyResults[4], "Success 3-3");
    test_equals(gSpyResults[5], "Success 4-1");
    test_equals(gSpyResults[6], "Success 4-2");
    test_equals(gSpyResults[7], "Success 4-3");
    gSpyResults.clear();

    // Do some nested blocks

    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile("if true; if false; spy('Error!'); else; spy('Nested 1'); end;"
                "else; spy('Error!'); end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 1");
    gSpyResults.clear();

    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile("if false; spy('Error!'); else; if false; spy('Error!');"
                "else; spy('Nested 2'); end; end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 2");
    gSpyResults.clear();
    
    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile("if false; spy('Error!');"
                "else; if true; spy('Nested 3'); else; spy('Error!'); end; end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 3");
    gSpyResults.clear();

    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile("if true; if false; spy('Error!'); else; spy('Nested 4'); end;"
                "else; spy('Error!'); end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 4");
    gSpyResults.clear();

    branch.clear();
    import_function(branch, spy_function, "spy(string)");
    branch.compile(
    "if (false)\n"
        "spy('Error!')\n"
    "else\n"
        "if (true)\n"
            "if (false)\n"
                "spy('Error!')\n"
            "else\n"
                "if (false)\n"
                    "spy('Error!')\n"
                "else\n"
                    "if (true)\n"
                        "spy('Nested 5')\n"
                    "else\n"
                        "spy('Error!')\n"
                    "end\n"
                "end\n"
            "end\n"
        "else\n"
            "spy('Error!')\n"
        "end\n"
    "end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 5");
    gSpyResults.clear();
}

void test_execution_with_elif()
{
    Branch branch;
    import_function(branch, spy_function, "spy(string)");
    gSpyResults.clear();

    branch.compile("x = 5");

    branch.compile("if x > 5; spy('Fail');"
                "elif x < 5; spy('Fail');"
                "elif x == 5; spy('Success');"
                "else; spy('Fail'); end");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Success");
    gSpyResults.clear();
}

void test_parse_with_no_line_endings()
{
    Branch branch;

    branch.compile("a = 4");
    branch.compile("if a < 5 a = 5 end");
    branch.compile("a=a");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 5);

    branch.compile("if a > 7 a = 5 else a = 3 end");
    branch.compile("a=a");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 3);

    branch.compile("if a == 2 a = 1 elif a == 3 a = 9 else a = 2 end");
    branch.compile("a=a");
    evaluate_branch(branch);
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 9);
}

void test_state_simple()
{
    Branch branch;
    EvalContext context;

    // Simple test, condition never changes
    Term* block = branch.compile("if true; state i = 0; i += 1; end");
    evaluate_branch(&context, branch);

    TaggedValue *i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(i != NULL);
    test_assert(as_int(i) == 1);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(0)->getField("i");
    test_assert(as_int(i) == 3);

    // Same test with elif
    branch.clear();
    block = branch.compile("if false; elif true; state i = 0; i += 1; end");
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 1);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 3);

    // Same test with else
    branch.clear();
    context = EvalContext();
    block = branch.compile("if false; else state i = 0; i += 1; end");
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 1);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 2);
    evaluate_branch(&context, branch);
    i = context.state.getField("_if_block")->getIndex(1)->getField("i");
    test_assert(as_int(i) == 3);
}

void test_state_in_function()
{
    // Use state in an if block in a function, this should verify that state
    // is being swapped in and out.
    Branch branch;
    EvalContext context;

    Term* my_func = branch.compile("def my_func() -> int;"
           " if true; state i = 0; i += 1; return(i); else return(0) end end");

    test_assert(is_function_stateful(my_func));

    Term* call1 = branch.compile("my_func()");

    test_assert(as_int(call1) == 0);

    evaluate_branch(&context, branch);
    evaluate_branch(&context, branch);
    evaluate_branch(&context, branch);

    test_assert(context);

    test_equals(as_int(call1), 3);
}

void test_state_is_reset_when_if_fails()
{
    Branch branch;
    EvalContext context;

    Term* c = branch.compile("c = true");
    branch.compile("if c; state i = 0; i += 1; end");

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_if_block: [[i: 1], null]]");

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_if_block: [[i: 2], null]]");

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_if_block: [[i: 3], null]]");

    set_bool(c, false);

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_if_block: [null, []]]");

    set_bool(c, true);

    evaluate_branch(&context, branch);
    test_equals(&context.state, "[_if_block: [[i: 1], null]]");
}

void test_nested_state()
{
    Branch branch;
    EvalContext context;

    branch.compile("t = false; if true; t = toggle(true); end");
    TaggedValue* t = get_local(branch["t"]);

    evaluate_branch(&context, branch);
    test_assert(as_bool(t) == true);
    evaluate_branch(&context, branch);
    test_assert(as_bool(t) == false);
    evaluate_branch(&context, branch);
    test_assert(as_bool(t) == true);
    evaluate_branch(&context, branch);
    test_assert(as_bool(t) == false);
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
    REGISTER_TEST_CASE(if_block_tests::test_state_in_function);
    REGISTER_TEST_CASE(if_block_tests::test_state_is_reset_when_if_fails);
    REGISTER_TEST_CASE(if_block_tests::test_nested_state);
}

} // namespace if_block_tests
} // namespace circa
