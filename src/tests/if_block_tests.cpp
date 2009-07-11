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

void register_tests()
{
    REGISTER_TEST_CASE(if_block_tests::test_if_joining);
    REGISTER_TEST_CASE(if_block_tests::test_if_joining_on_bool);
}

} // namespace if_block_tests
} // namespace circa
