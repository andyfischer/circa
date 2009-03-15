// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace ref_tests {

void test_basic()
{
    Ref ref;

    test_assert(ref._t == NULL);

    Term* t = new Term();

    test_assert(t->refs.size() == 0);

    ref = t;

    test_assert(ref._t == t);
    test_assert(t->refs.size() == 1);
    test_assert(t->refs[0] == &ref);
    test_assert( ((Term*) ref) == t);

    ref = NULL;

    test_assert(ref._t == NULL);
    test_assert(t->refs.size() == 0);
}

void test_copy()
{
    Ref ref;

    Term* t = new Term();

    ref = t;

    Ref copy(ref);

    test_assert(copy._t == t);
    test_assert(t->refs.size() == 2);
    test_assert(t->refs[0] == &ref);
    test_assert(t->refs[1] == &copy);

    Ref another_copy(NULL);
    another_copy = copy;
    test_assert(another_copy._t == t);
    test_assert(t->refs.size() == 3);
    test_assert(t->refs[2] == &another_copy);
}

void test_destroy()
{
    Term* t = new Term();

    {
        Ref ref;
        ref = t;
        test_assert(t->refs.size() == 1);
    }

    test_assert(t->refs.size() == 0);
}

void test_list()
{
    RefList list;

    Term* t1 = new Term();
    Term* t2 = new Term();

    list.append(t1);
    list.append(t2);

    test_assert(t1->refs.size() == 1);
    test_assert(t2->refs.size() == 1);

    RefList another_list = list;
    test_assert(t1->refs.size() == 2);
    test_assert(t2->refs.size() == 2);

    {
        RefList a_third_list = list;
        test_assert(t1->refs.size() == 3);
        test_assert(t2->refs.size() == 3);
    }

    test_assert(t1->refs.size() == 2);
    test_assert(t2->refs.size() == 2);
}

void test_safe_term_deletion()
{
    Term* t = new Term();

    Ref ref(t);
    RefList list(t,t);

    test_assert(t->refs[0] == &ref);

    delete t;

    test_assert(ref._t == NULL);
    test_assert(list[0] == NULL);
    test_assert(list[1] == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
    REGISTER_TEST_CASE(ref_tests::test_destroy);
    REGISTER_TEST_CASE(ref_tests::test_list);
    REGISTER_TEST_CASE(ref_tests::test_safe_term_deletion);
}

} // namespace ref_tests

} // namespace circa
