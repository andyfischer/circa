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

    ref = NULL;

    test_assert(ref._target == NULL);
    test_assert(t->refs.size() == 0);
}

void test_copy()
{

}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_basic);
    REGISTER_TEST_CASE(ref_tests::test_copy);
}
} // namespace ref_tests

} // namespace circa
