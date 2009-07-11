// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace branch_tests {

void test_remove()
{
    Branch branch;

    create_value(branch, INT_TYPE, "a");

    test_assert(branch.length() == 1);
    test_assert(branch.contains("a"));

    branch.remove(0);

    test_assert(branch.length() == 0);
    test_assert(!branch.contains("a"));
}

void test_duplicate()
{
    Branch original;
    Term* term1 = int_value(original, 5);
    Term* term2 = string_value(original, "yarn");
    original.bindName(term1, "term1");
    original.bindName(term2, "term two");

    Branch duplicate;
    duplicate_branch(original, duplicate);

    Term* term1_duplicate = duplicate["term1"];
    test_assert(term1_duplicate != NULL);

    Term* term2_duplicate = duplicate["term two"];

    test_assert(as_int(term1_duplicate) == 5);
    test_assert(as_string(term2_duplicate) == "yarn");

    // make sure 'duplicate' uses different terms
    as_int(term1) = 8;
    test_assert(as_int(term1_duplicate) == 5);

    sanity_check_term(term1);
    sanity_check_term(term2);
    sanity_check_term(term1_duplicate);
    sanity_check_term(term2_duplicate);
}

void test_duplicate_nested()
{
    Branch branch;
    branch.eval("a = 1.0");
    Branch& inner = branch.eval("inner = branch()")->asBranch();
    inner.eval("i = 2.0");
    inner.eval("j = add(a,i)");

    Branch dupe;
    duplicate_branch(branch, dupe);

    Term* inner_i = dupe["inner"]->field("i");
    Term* inner_j = dupe["inner"]->field("j");

    test_assert(inner_i != NULL);
    test_assert(inner_j != NULL);

    test_assert(dupe["a"]->asFloat() == 1.0);
    test_assert(inner_i->asFloat() == 2.0);
    test_assert(inner_j->input(0) != branch["a"]);
    test_assert(inner_j->input(0) != NULL);
    //test_assert(inner_j->input(0) == dupe["a"]);
    //test_assert(inner_j->input(0)->input(0) == dupe["a"]);
    test_assert(inner_j->input(1) == inner_i);
}

void test_duplicate_nested_dont_make_extra_terms()
{
    // this test case looks for a bug where nested branches have
    // too many terms after duplication
    Branch orig;
    Branch& inner = as_branch(orig.eval("inner = branch()"));
    inner.eval("i = 2");

    Branch dupe;
    duplicate_branch(orig, dupe);

    test_assert(dupe["inner"]->asBranch().length() == 1);
}

void test_duplicate_subroutine()
{
    Branch branch;

    Term* func = branch.eval("def func()\na = 1\nend");

    // sanity check:
    test_assert(function_get_name(func) == "func");
    test_assert(branch.contains("func"));

    Branch dupe;
    duplicate_branch(branch, dupe);

    test_assert(dupe.contains("func"));

    Term* dupedFunc = dupe["func"];
    test_assert(function_get_name(dupedFunc) == "func");

    test_assert(is_branch(dupedFunc));

    test_assert(as_branch(func).length() == as_branch(dupedFunc).length());
    test_assert(as_branch(func)[1]->function == as_branch(dupedFunc)[1]->function);
    test_assert(as_branch(func)[1]->type == as_branch(dupedFunc)[1]->type);
    test_assert(as_branch(func)[1]->asInt() == as_branch(dupedFunc)[1]->asInt());
    test_assert(as_branch(dupedFunc)[1] == as_branch(dupedFunc)["a"]);
}

void test_duplicate_get_field_by_name()
{
    Branch branch;
    branch.eval("type mytype { int f }");
    branch.eval("v = mytype()");
    Term* b = branch.eval("b = v.f");

    test_assert(b->function == GET_FIELD_FUNC);

    Branch duped_branch;
    duplicate_branch(branch, duped_branch);

    b = duped_branch["b"];

    test_assert(b->function == GET_FIELD_FUNC);
}

void find_name_in_outer_branch()
{
    Branch branch;
    Term* outer_branch = branch.eval("Branch()");
    alloc_value(outer_branch);

    Term* a = as_branch(outer_branch).eval("a = 1");

    Term* inner_branch = as_branch(outer_branch).eval("Branch()");
    alloc_value(inner_branch);

    test_assert(find_named(as_branch(inner_branch), "a") == a);
}

void test_migrate()
{
    Branch dest, source;

    Term* a = dest.eval("state a = 1");
    source.eval("state a = 2");

    migrate_stateful_values(source, dest);

    // Test that the 'dest' terms are the same terms
    test_assert(dest["a"] == a);

    // Test that the value was transferred
    test_assert(dest["a"]->asInt() == 2);
}

void test_migrate2()
{
    // In this test, we migrate with 1 term added and 1 term removed.
    Branch source, dest;

    Term* a = dest.eval("state a = 1");
    /*Term* b =*/ dest.eval("state b = 2");

    /*Term* a2 =*/ source.eval("state a = 3");
    /*Term* c =*/ source.eval("state c = 4");

    migrate_stateful_values(source, dest);

    test_assert(dest["a"] == a);
    test_assert(dest["a"]->asInt() == 3);
}

void test_assign()
{
    Branch branch;

    branch.eval("type Mytype { int a, float b }");

    // Assign a value that will require coercion
    Term* dest = branch.eval("Mytype()");
    Term* source = branch.eval("[3 4]");
    Term* dest0 = as_branch(dest)[0];
    Term* dest1 = as_branch(dest)[1];

    assign_value(source, dest);
    test_assert(is_int(dest0));
    test_assert(as_int(dest0) == 3);
    test_assert(is_float(dest1));
    test_equals(as_float(dest1), 4);
    // Verify that 'dest' has the same terms
    test_assert(dest0 == as_branch(dest)[0]);
    test_assert(dest1 == as_branch(dest)[1]);

    // Assign a value with more elements
    dest = branch.eval("[1]");
    dest0 = as_branch(dest)[0];
    test_assert(as_int(dest0) == 1);

    source = branch.eval("[7 8 9]");

    assign_value(source, dest);

    test_assert(as_branch(dest).length() == 3);
    test_assert(dest0 == as_branch(dest)[0]);
    test_assert(as_int(dest0) == 7);
    test_assert(as_int(as_branch(dest)[1]) == 8);
    test_assert(as_int(as_branch(dest)[2]) == 9);
}

void register_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_remove);
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_nested);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_nested_dont_make_extra_terms);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_subroutine);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_get_field_by_name);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
    REGISTER_TEST_CASE(branch_tests::test_migrate);
    REGISTER_TEST_CASE(branch_tests::test_migrate2);
    REGISTER_TEST_CASE(branch_tests::test_assign);
}

} // namespace branch_tests

} // namespace circa
