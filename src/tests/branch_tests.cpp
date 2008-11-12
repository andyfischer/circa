// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "testing.h"
#include "branch.h"
#include "builtins.h"
#include "introspection.h"
#include "runtime.h"
#include "term.h"

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

} // namespace branch_tests

void register_branch_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::external_pointers);
}

} // namespace circa
