// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace ref_tests {

void test_basic()
{
    Ref ref;

    test_assert(ref.t == NULL);

    Term* t = new Term();

    test_assert(t->refCount == 0);

    ref = t;

    test_assert(ref.t == t);
    test_assert(t->refCount == 1);
    test_assert( ((Term*) ref) == t);

    ref = NULL;

    test_assert(ref.t == NULL);
}

void test_copy()
{
    Ref ref;

    Term* t = new Term();

    ref = t;

    Ref copy(ref);

    test_assert(copy.t == t);
    test_assert(t->refCount == 2);

    Ref another_copy(NULL);
    another_copy = copy;
    test_assert(another_copy.t == t);
    test_assert(t->refCount == 3);
}

void test_destroy()
{
    Term* t = new Term();
    Ref outerref(t);

    {
        Ref ref;
        ref = t;
        test_assert(t->refCount == 2);
    }

    test_assert(t->refCount == 1);
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
    REGISTER_TEST_CASE(ref_tests::test_destroy);
}

} // namespace ref_tests

} // namespace circa
