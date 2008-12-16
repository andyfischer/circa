// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
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
    original.bindName(term1, "one name for term1");
    original.bindName(term1, "another name for term1");
    original.bindName(term2, "term two");

    Branch duplicate;

    duplicate_branch(&original, &duplicate);

    Term* term1_duplicate = duplicate.getNamed("one name for term1");
    test_assert(term1_duplicate != NULL);
    test_assert(term1_duplicate == duplicate.getNamed("another name for term1"));

    Term* term2_duplicate = duplicate.getNamed("term two");

    test_assert(as_int(term1_duplicate) == 5);
    test_assert(as_string(term2_duplicate) == "yarn");

    // make sure 'duplicate' uses different terms
    as_int(term1) = 8;
    test_assert(as_int(term1_duplicate) == 5);
}

void external_pointers()
{
    Branch branch;

    Term* inner_branch = create_var(&branch, BRANCH_TYPE);

    test_equals(list_all_pointers(inner_branch),
        ReferenceList(inner_branch->function, inner_branch->type));

    Term* inner_int = create_var(&as_branch(inner_branch), INT_TYPE);
    Term* inner_add = apply_function(as_branch(inner_branch), ADD_FUNC,
            ReferenceList(inner_int, inner_int));

    // make sure that the pointer from inner_add to inner_int does
    // not show up in list_all_pointers.

    test_equals(list_all_pointers(inner_branch), ReferenceList(
                inner_branch->function,
                inner_branch->type,
                inner_int->function,
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
                INT_TYPE,
                MULT_FUNC,
                FLOAT_TYPE));

    test_assert(inner_add->function == MULT_FUNC);
}

void test_owning_term()
{
    Branch branch;

    Term* b = eval_statement(branch, "Branch()");
    alloc_value(b);

    test_assert(b->type == BRANCH_TYPE);
    test_assert(as_branch(b).owningTerm == b);

    Term* b2 = eval_statement(branch, "Branch()");
    steal_value(b, b2);

    test_assert(as_branch(b2).owningTerm == b2);

    duplicate_value(b2, b);

    test_assert(as_branch(b).owningTerm == b);
}

void find_name_in_outer_branch()
{
    Branch branch;
    Term* a = eval_statement(branch, "a = 1");

    Term* b = eval_statement(branch, "Branch()");
    alloc_value(b);

    test_assert(as_branch(b).findNamed("a") == a);
}

void find_name_in_outer_branch_from_if()
{
    Branch branch;
    Term* a = eval_statement(branch, "a = 1");

    Term* if_s = eval_function(branch, "if-statement", ReferenceList(CONSTANT_TRUE));

    Branch& posBranch = as_branch(if_s->state->field(0));

    test_assert(posBranch.owningTerm == if_s->state);
    test_assert(if_s->state->owningBranch == if_s->myBranch);
    test_assert(if_s->myBranch->owningTerm == if_s);
    test_assert(if_s->owningBranch == &branch);

    test_assert(posBranch.findNamed("a") == a);
}

} // namespace branch_tests

void register_branch_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::external_pointers);
    REGISTER_TEST_CASE(branch_tests::test_owning_term);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch_from_if);
}

} // namespace circa
