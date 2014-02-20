// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "handle.h"
#include "kernel.h"
#include "importing.h"
#include "tagged_value.h"
#include "type.h"

namespace handle_test {

void test_value_is_shared()
{
    // this once did something
}

int gTimesReleaseCalled = 0;

void my_release_func(caValue* value, void* ptr)
{
    gTimesReleaseCalled++;
}

void register_tests()
{
    REGISTER_TEST_CASE(handle_test::test_value_is_shared);
}

} // namespace handle_test
