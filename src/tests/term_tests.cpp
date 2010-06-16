// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace term_tests {

void simple_refcounting()
{
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
}

void remove_user_reference_on_delete()
{
    Branch branch1;
    Term* term1 = branch1.appendNew();
    Ref ref2;

    {
        Branch branch2;
        ref2 = branch2.appendNew();
        test_assert(term1->refCount == 1);
        test_assert(ref2->refCount == 2);

        set_input(ref2, 0, term1);
        test_assert(term1->refCount == 2);
        test_assert(ref2->refCount == 3);
        test_assert(term1->users[0] == ref2);
    }

    // The 'user' reference should be removed here.
    test_assert(term1->users.length() == 0);
    test_assert(ref2->refCount == 1);

    // term2 is now an orphaned term
    test_assert(ref2->owningBranch == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(term_tests::simple_refcounting);
    REGISTER_TEST_CASE(term_tests::remove_user_reference_on_delete);
}

} // namespace term_tests
} // namespace circa
