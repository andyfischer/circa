// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "debug.h"
#include "introspection.h"
#include "parser.h"
#include "runtime.h"
#include "term.h"
#include "testing.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace branch_tests {

void test_duplicate()
{
    Branch original;
    Term* term1 = apply_function(&original, VAR_INT, ReferenceList());
    Term* term2 = apply_function(&original, VAR_STRING, ReferenceList());
    as_int(term1) = 5;
    as_string(term2) = "yarn";
    original.bindName(term1, "term1");
    original.bindName(term2, "term two");

    Branch duplicate;

    duplicate_branch(original, duplicate);

    Term* term1_duplicate = duplicate.getNamed("term1");
    test_assert(term1_duplicate != NULL);

    Term* term2_duplicate = duplicate.getNamed("term two");

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
    Branch& inner = branch.eval("inner = branch()")->state->asBranch();
    inner.outerScope = &branch;
    inner.eval("i = 2.0");
    inner.eval("j = add(a,i)");

    Branch dupe;
    duplicate_branch(branch, dupe);

    Term* inner_i = dupe["inner"]->state->asBranch()["i"];
    Term* inner_j = dupe["inner"]->state->asBranch()["j"];

    test_assert(inner_i != NULL);
    test_assert(inner_j != NULL);

    test_assert(dupe["a"]->asFloat() == 1.0);
    test_assert(inner_i->asFloat() == 2.0);
    test_assert(inner_j->input(0) != branch["a"]);
    test_assert(inner_j->input(0) != NULL);
    test_assert(inner_j->input(0) == dupe["a"]);
    test_assert(inner_j->input(1) == inner_i);
}

void find_name_in_outer_branch()
{
    Branch branch;
    Term* outer_branch = branch.eval("Branch()");
    alloc_value(outer_branch);

    Term* a = as_branch(outer_branch).eval("a = 1");

    Term* inner_branch = as_branch(outer_branch).eval("Branch()");
    alloc_value(inner_branch);

    as_branch(inner_branch).outerScope = &as_branch(outer_branch);

    test_assert(find_named(&as_branch(inner_branch), "a") == a);
}

void cleanup_users_after_delete()
{
    Branch branch;

    Term* a = branch.eval("a = 1.0");

    Branch &subBranch = as_branch(branch.eval("sub = Branch()"));
    subBranch.outerScope = &branch;

    Term* b = subBranch.eval("b = add(a,a)");

    test_assert(a->users.contains(b));

    dealloc_value(branch["sub"]);

    test_assert(!a->users.contains(b));
}

void test_startBranch()
{
    Branch branch;

    Term* a = branch.eval("a = 1");

    Branch& sub = branch.startBranch("sub");

    test_assert(find_named(&sub, "a") == a);
}

void test_migrate()
{
    Branch orig, replace;

    Term* a = orig.eval("a = 1");

    replace.eval("a = 2");

    migrate_branch(replace, orig);

    // Test that the 'orig' terms are the same terms
    test_assert(orig["a"] == a);

    // Test that the value was transferred
    test_assert(as_int(a) == 2);
}

void test_migrate2()
{
    // In this test, we migrate with 1 term added and 1 term removed.
    Branch orig, replace;

    Term* a = orig.eval("a = 1");
    Term* b = orig.eval("b = 2");

    Term* a2 = replace.eval("a = 3");
    Term* c = replace.eval("c = 4");

    // Sanity check our branches
    test_assert(orig[0] == a);
    test_assert(orig[1] == b);
    test_assert(replace[0] == a2);
    test_assert(replace[1] == c);

    migrate_branch(replace, orig);

    test_assert(orig[0] == a);
    test_assert(orig[1]->name == "c");
    test_assert(as_int(orig[0]) == 3);
}

void test_dont_migrate_state()
{
    Branch orig, replace;
    Term* a = orig.eval("state int a = 1");
    replace.eval("a = 2");

    test_assert(a->isStateful());

    migrate_branch(replace, orig);

    test_assert(orig["a"] == a);
    test_assert(as_int(orig["a"]) == 1);

    orig.clear();
    replace.clear();

    a = orig.eval("state int a = 1");
    replace.eval("state int a = 2");
    test_assert(orig["a"] == a);
    test_assert(as_int(orig["a"]) == 1);
}


void register_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_nested);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
#if TRACK_USERS
    REGISTER_TEST_CASE(branch_tests::cleanup_users_after_delete);
#endif
    REGISTER_TEST_CASE(branch_tests::test_startBranch);
    REGISTER_TEST_CASE(branch_tests::test_migrate);
    REGISTER_TEST_CASE(branch_tests::test_migrate2);
    REGISTER_TEST_CASE(branch_tests::test_dont_migrate_state);
}

} // namespace branch_tests

} // namespace circa
