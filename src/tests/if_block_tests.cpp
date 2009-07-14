// Copyright 2008 Andrew Fischer

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

void register_tests()
{
    REGISTER_TEST_CASE(if_block_tests::test_if_joining);
    REGISTER_TEST_CASE(if_block_tests::test_if_joining);
    REGISTER_TEST_CASE(if_block_tests::test_if_elif_else);
    REGISTER_TEST_CASE(if_block_tests::test_dont_always_rebind_inner_names);
}

} // namespace if_block_tests
} // namespace circa
