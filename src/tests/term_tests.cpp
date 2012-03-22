// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circ_internal.h"

namespace circa {
namespace term_tests {

void simple_refcounting()
{
    #if 0
    TEST_DISABLED
    Term* term = NULL;
    Ref ref;

    {
        Branch myBranch;
        term = myBranch.appendNew();
        test_assert(term->refCount == 1);
        ref = term;
        test_assert(term->refCount == 2);
    }

    test_assert(term->refCount == 1);
    #endif
}

void duplicate_nested_contents()
{
    Branch branch;
    Term* a = branch.compile("a = branch()");

    Term* x = create_int(nested_contents(a), 5);

    // Try duplicating branch
    Branch branch2;
    duplicate_branch(&branch, &branch2);

    test_assert(branch2.length() == 1);
    test_assert(branch2[0]->name == "a");
    test_assert(branch2[0]->contents()->length() == 1);
    test_assert(branch2[0]->contents()->get(0)->asInt() == 5);

    branch2.clear();

    // Now try duplicating, check that internal references are updated
    apply(nested_contents(a), "add", TermList(x, x));
    duplicate_branch(&branch, &branch2);

    test_assert(branch2.length() == 1);
    test_assert(branch2[0]->name == "a");
    test_assert(branch2[0]->contents()->length() == 2);
    test_assert(branch2[0]->contents()->get(0)->asInt() == 5);
    test_assert(branch2[0]->contents()->get(1)->input(0) == branch2[0]->contents()->get(0));
    test_assert(branch2[0]->contents()->get(1)->input(1) == branch2[0]->contents()->get(0));
}

void register_tests()
{
    REGISTER_TEST_CASE(term_tests::simple_refcounting);
    REGISTER_TEST_CASE(term_tests::duplicate_nested_contents);
}

} // namespace term_tests
} // namespace circa
