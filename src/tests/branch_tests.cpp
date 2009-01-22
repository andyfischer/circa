// Copyright 2008 Paul Hodge

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
    Term* term1 = apply_function(original, VAR_INT, ReferenceList());
    Term* term2 = apply_function(original, VAR_STRING, ReferenceList());
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

    assert(sanity_check_term(term1));
    assert(sanity_check_term(term2));
    assert(sanity_check_term(term1_duplicate));
    assert(sanity_check_term(term2_duplicate));
}

void external_pointers()
{
    Branch branch;

    Term* inner_branch = create_value(&branch, BRANCH_TYPE);

    test_equals(list_all_pointers(inner_branch),
        ReferenceList(inner_branch->function, inner_branch->type));

    Term* inner_int = create_value(&as_branch(inner_branch), INT_TYPE);
    Term* inner_add = apply_function(as_branch(inner_branch), ADD_FUNC,
            ReferenceList(inner_int, inner_int));

    // make sure that the pointer from inner_add to inner_int does
    // not show up in list_all_pointers.

    /*
     fixme
    test_equals(list_all_pointers(inner_branch), ReferenceList(
                inner_branch->function,
                inner_branch->type,
                inner_int->function,
                FLOAT_TYPE,
                INT_TYPE,
                ADD_FUNC,
                FLOAT_TYPE));

    ReferenceMap myRemap;
    myRemap[ADD_FUNC] = MULT_FUNC;

    remap_pointers(inner_branch, myRemap);

    test_equals(list_all_pointers(inner_branch), ReferenceList(
                inner_branch->function,
                inner_branch->type,
                inner_int->function,
                FLOAT_TYPE,
                INT_TYPE,
                MULT_FUNC,
                FLOAT_TYPE));

    test_assert(inner_add->function == MULT_FUNC);
    */
}

/*
void test_owning_term()
{
    Branch branch;

    Term* b = branch.eval("Branch()");
    alloc_value(b);

    test_assert(b->type == BRANCH_TYPE);
    test_assert(as_branch(b).owningTerm == b);

    Term* b2 = branch.eval("Branch()");
    steal_value(b, b2);

    test_assert(as_branch(b2).owningTerm == b2);

    duplicate_value(b2, b);

    test_assert(as_branch(b).owningTerm == b);
}*/

void find_name_in_outer_branch()
{
    Branch branch;
    Term* outer_branch = branch.eval("Branch()");
    alloc_value(outer_branch);

    Term* a = as_branch(outer_branch).eval("a = 1");

    Term* inner_branch = as_branch(outer_branch).eval("Branch()");
    alloc_value(inner_branch);

    as_branch(inner_branch).outerScope = &as_branch(outer_branch);

    test_assert(as_branch(inner_branch).findNamed("a") == a);
}

void test_startBranch()
{
    Branch branch;

    Term* a = branch.eval("a = 1");

    Branch& sub = branch.startBranch("sub");

    test_assert(sub.findNamed("a") == a);
}

void test_migrate()
{
    Branch orig, replace;

    Term* a = orig.eval("a = 1");

    replace.eval("a = 2");

    migrate_branch(orig, replace);

    // Test that the 'orig' terms are the same terms
    test_assert(orig["a"] == a);

    // Test that the value was transferred
    test_assert(as_int(a) == 2);
}

} // namespace branch_tests

void register_branch_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::external_pointers);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
    REGISTER_TEST_CASE(branch_tests::test_startBranch);
    REGISTER_TEST_CASE(branch_tests::test_migrate);
}

} // namespace circa
