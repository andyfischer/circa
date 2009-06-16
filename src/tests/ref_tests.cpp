// Copyright 2008 Andrew Fischer

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

void test_list()
{
    RefList list;

    Term* t1 = new Term();
    Term* t2 = new Term();

    list.append(t1);
    list.append(t2);

    test_assert(t1->refCount == 1);
    test_assert(t2->refCount == 1);

    RefList another_list = list;
    test_assert(t1->refCount == 2);
    test_assert(t2->refCount == 2);

    {
        RefList a_third_list = list;
        test_assert(t1->refCount == 3);
        test_assert(t2->refCount == 3);
    }

    test_assert(t1->refCount == 2);
    test_assert(t2->refCount == 2);
}

void remap_properties()
{
    // TODO
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
    REGISTER_TEST_CASE(ref_tests::test_destroy);
    REGISTER_TEST_CASE(ref_tests::test_list);
}

} // namespace ref_tests

} // namespace circa
