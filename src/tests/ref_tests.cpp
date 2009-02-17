// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace ref_tests {

void test_basic()
{
    Ref ref(NULL);

    test_assert(ref._target == NULL);
    test_assert(ref._owner == NULL);

    Term* t = new Term();

    test_assert(t->refs.size() == 0);

    ref = t;

    test_assert(ref._target == t);
    test_assert(t->refs.size() == 1);
    test_assert(t->refs[0] == &ref);
    test_assert( ((Term*) ref) == t);

    ref = NULL;

    test_assert(ref._target == NULL);
    test_assert(t->refs.size() == 0);


    delete t;
}

void test_copy()
{
    Ref ref(NULL);

    Term* t = new Term();

    ref = t;

    Ref copy(ref);

    test_assert(copy._target == t);
    test_assert(t->refs.size() == 2);
    test_assert(t->refs[0] == &ref);
    test_assert(t->refs[1] == &copy);

    Ref another_copy(NULL);
    another_copy = copy;
    test_assert(another_copy._target == t);
    test_assert(t->refs.size() == 3);
    test_assert(t->refs[2] == &another_copy);

    delete t;
}

void test_destroy()
{
    Term* t = new Term();

    {
        Ref ref(NULL);
        ref = t;
        test_assert(t->refs.size() == 1);
    }

    test_assert(t->refs.size() == 0);
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
    REGISTER_TEST_CASE(ref_tests::test_destroy);
}
} // namespace ref_tests

} // namespace circa
