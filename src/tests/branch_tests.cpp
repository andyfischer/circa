// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <tests/common.h>
#include "branch.h"
#include "builtins.h"
#include "operations.h"
#include "term.h"

namespace circa {
namespace branch_tests {

void test_duplicate()
{
    Branch original;
    Term* term1 = apply_function(&original, CONSTANT_INT, ReferenceList());
    Term* term2 = apply_function(&original, CONSTANT_STRING, ReferenceList());
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

} // namespace branch_tests

void register_branch_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
}

} // namespace circa
