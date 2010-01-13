// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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
    Ref t1 = alloc_term();
    Ref t2 = alloc_term();

    test_assert(t1->refCount == 1);

    {
        RefList list;
        list.append(t1);
        list.append(t2);

        test_assert(t1->refCount == 2);
        test_assert(t2->refCount == 2);

        {
            RefList another_list = list;
            test_assert(t1->refCount == 3);
            test_assert(t2->refCount == 3);

            {
                RefList a_third_list = list;
                test_assert(t1->refCount == 4);
                test_assert(t2->refCount == 4);
            }

            test_assert(t1->refCount == 3);
            test_assert(t2->refCount == 3);
        }

        test_assert(t1->refCount == 2);
        test_assert(t2->refCount == 2);
    }

    test_assert(t1->refCount == 1);
}

void remap_properties()
{
    Branch branch;
    Term* a = create_int(branch, 1, "a");
    Term* b = create_int(branch, 2, "b");
    b->setRefProp("test_property", a);

    Branch duplicate;
    duplicate_branch(branch, duplicate);

    test_assert(duplicate["b"]->refProp("test_property") == duplicate["a"]);
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
    REGISTER_TEST_CASE(ref_tests::test_destroy);
    REGISTER_TEST_CASE(ref_tests::test_list);
    REGISTER_TEST_CASE(ref_tests::remap_properties);
}

} // namespace ref_tests

} // namespace circa
