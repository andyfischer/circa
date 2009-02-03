// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "circa.h"
#include "testing.h"

namespace circa {
namespace value_tests {

void test_duplicate()
{
    Branch branch;

    Term* a = create_value(&branch, INT_TYPE);
    Term* b = create_value(&branch, INT_TYPE);

    as_int(a) = 2;
    as_int(b) = 3;

    void* a_addr = a->value;
    //void* b_addr = b->value;

    copy_value(a, b);

    test_assert(as_int(b) == 2);
    test_assert(b->value != a_addr);
    // This check doesn't work because it's possible for the newly
    // created data to have the same address as the old data. But
    // it would be good to test that there really is a reallocation
    // going on.
    //test_assert(b->value != b_addr);
}

void test_assign()
{
    Branch branch;

    Term* a = create_value(&branch, INT_TYPE);
    Term* b = create_value(&branch, INT_TYPE);

    as_int(a) = 2;
    as_int(b) = 3;

    void* a_addr = a->value;
    void* b_addr = b->value;

    copy_value(a, b);

    test_assert(as_int(b) == 2);
    test_assert(b->value != a_addr);
    test_assert(b->value == b_addr);
}

void register_tests()
{
    REGISTER_TEST_CASE(value_tests::test_duplicate);
    REGISTER_TEST_CASE(value_tests::test_assign);
}
} // namespace value_tests

} // namespace circa
