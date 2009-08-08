// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace if_block_tests {

void test_if_joining()
{
    Branch branch;

    // Test that a name defined in one branch is not rebound in outer scope
    branch.eval("if true\napple = 5\nend");
    test_assert(!branch.contains("apple"));

    // Test that a name which exists in the outer scope is rebound
    Term* original_banana = int_value(branch, 10, "banana");
    branch.eval("if true\nbanana = 15\nend");
    test_assert(branch["banana"] != original_banana);

    // Test that if a name is defined in both 'if' and 'else branches, that it gets defined 
    // in the outer scope.
    branch.eval("if true\nCardiff = 5\nelse\nCardiff = 11\nend");
    test_assert(branch.contains("Cardiff"));
}

void test_if_joining_on_bool()
{
    // The following code once had a bug where if_expr wouldn't work
    // if one of its inputs was missing value.
    Branch branch;
    Term* s = branch.eval("hey = true");

    test_assert(s->value != NULL);

    branch.eval("if false\nhey = false\nend");

    evaluate_branch(branch);

    test_assert(branch["hey"]->asBool() == true);
}

void test_if_elif_else()
{
    Branch branch;

    branch.eval("if true; a = 1; elif true; a = 2; else; a = 3; end");

    test_assert(branch.contains("a"));
    test_assert(branch["a"]->asInt() == 1);

    branch.eval("if false; b = 'apple'; elif false; b = 'orange'; else; b = 'pineapple'; end");
    test_assert(branch.contains("b"));
    test_assert(branch["b"]->asString() == "pineapple");

    // try one without 'else'
    branch.clear();
    branch.eval("c = 0");
    branch.eval("if false; c = 7; elif true; c = 8; end");
    test_assert(branch.contains("c"));
    test_assert(branch["c"]->asInt() == 8);

    // try with some more complex conditions
    branch.clear();
    branch.eval("x = 5");
    branch.eval("if x > 6; compare = 1; elif x < 6; compare = -1; else; compare = 0; end");

    test_assert(branch.contains("compare"));
    test_assert(branch["compare"]->asInt() == -1);
}

void test_dont_always_rebind_inner_names()
{
    Branch branch;
    branch.eval("if false; b = 1; elif false; c = 1; elif false; d = 1; else; e = 1; end");
    test_assert(!branch.contains("b"));
    test_assert(!branch.contains("c"));
    test_assert(!branch.contains("d"));
    test_assert(!branch.contains("e"));
}

std::vector<std::string> gSpyResults;

void spy_function(Term* caller)
{
    gSpyResults.push_back(as_string(caller->input(0)));
}

void test_execution()
{
    Branch branch;
    import_function(branch, spy_function, "spy(string)");
    gSpyResults.clear();

    // Start off with some simple expressions
    branch.eval("if true\nspy('Success 1')\nend");
    branch.eval("if false\nspy('Fail')\nend");
    branch.eval("if (1 + 2) > 1\nspy('Success 2')\nend");
    branch.eval("if (1 + 2) < 1\nspy('Fail')\nend");
    branch.eval("if true; spy('Success 3'); end");
    branch.eval("if false; spy('Fail'); end");

    test_assert(branch);
    test_assert(gSpyResults.size() == 3);
    test_equals(gSpyResults[0], "Success 1");
    test_equals(gSpyResults[1], "Success 2");
    test_equals(gSpyResults[2], "Success 3");
    gSpyResults.clear();
    
    // Use 'else'
    branch.eval("if true; spy('Success 1'); else; spy('Fail'); end");
    branch.eval("if false; spy('Fail'); else; spy('Success 2'); end");
    branch.eval("if true; spy('Success 3-1')\n spy('Success 3-2')\n spy('Success 3-3')\n"
                "else; spy('Fail'); end");
    branch.eval("if false; spy('Fail')\n spy('Fail 2')\n"
                "else; spy('Success 4-1')\n spy('Success 4-2')\n spy('Success 4-3')\n end");
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

    branch.eval("if true; if false; spy('Error!'); else; spy('Nested 1'); end;"
                "else; spy('Error!'); end");
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 1");
    gSpyResults.clear();

    branch.eval("if false; spy('Error!'); else; if false; spy('Error!');"
                "else; spy('Nested 2'); end; end");
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 2");
    gSpyResults.clear();
    
    branch.eval("if false; spy('Error!');"
                "else; if true; spy('Nested 3'); else; spy('Error!'); end; end");
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 3");
    gSpyResults.clear();

    branch.eval("if true; if false; spy('Error!'); else; spy('Nested 4'); end;"
                "else; spy('Error!'); end");
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Nested 4");
    gSpyResults.clear();

    branch.eval(
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

    branch.eval("x = 5");

    branch.eval("if x > 5; spy('Fail');"
                "elif x < 5; spy('Fail');"
                "elif x == 5; spy('Success');"
                "else; spy('Fail'); end");
    test_assert(branch);
    test_assert(gSpyResults.size() == 1);
    test_equals(gSpyResults[0], "Success");
    gSpyResults.clear();
}

void test_parse_with_no_line_endings()
{
    Branch branch;

    branch.eval("a = 4");
    branch.eval("if a < 5 a = 5 end");
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 5);

    branch.eval("if a > 7 a = 5 else a = 3 end");
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 3);

    branch.eval("if a == 2 a = 1 elif a == 3 a = 9 else a = 2 end");
    test_assert(branch);
    test_assert(branch["a"]->asInt() == 9);
}

void register_tests()
{
    REGISTER_TEST_CASE(if_block_tests::test_if_joining);
    REGISTER_TEST_CASE(if_block_tests::test_if_elif_else);
    REGISTER_TEST_CASE(if_block_tests::test_dont_always_rebind_inner_names);
    REGISTER_TEST_CASE(if_block_tests::test_execution);
    REGISTER_TEST_CASE(if_block_tests::test_execution_with_elif);
    REGISTER_TEST_CASE(if_block_tests::test_parse_with_no_line_endings);
}

} // namespace if_block_tests
} // namespace circa
