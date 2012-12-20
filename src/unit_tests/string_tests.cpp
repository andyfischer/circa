// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "string_type.h"

namespace string_tests {

void test_sneaky_equals()
{
    Value val1, val2;
    set_string(&val1, "Hello");
    set_string(&val2, "Hello");

    // initial: strings are stored differently.
    test_assert(circa_string(&val1) != circa_string(&val2));

    test_assert(circa_equals(&val1, &val2));

    // after equality check: strings are stored with same object.
    test_assert(circa_string(&val1) == circa_string(&val2));
}

void register_tests()
{
    REGISTER_TEST_CASE(test_sneaky_equals);
}

} // namespace string_tests
