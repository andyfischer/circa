// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "tagged_value.h"

namespace tagged_value_test {

void test_strict_equals()
{
    Value left, right;
    set_int(&left, 1);
    set_int(&right, 1);

    test_assert(strict_equals(&left, &right));

    set_string(&left, "abcde");
    test_assert(!strict_equals(&left, &right));

    set_string(&right, "abcde");
    test_assert(strict_equals(&left, &right));

    set_int(&left, 1);
    set_float(&right, 1.0);
    test_assert(!strict_equals(&left, &right));
}

void register_tests()
{
    REGISTER_TEST_CASE(tagged_value_test::test_strict_equals);
}

} // namespace tagged_value_test
