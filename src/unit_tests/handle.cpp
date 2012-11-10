// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

namespace handle {


void my_release(caValue* value)
{
    test_assert(gSlot[as_int(value)]);
    gSlot[as_int(value)] = false;
}

void test_simple()
{
    bool payload = false;

    circa::Value value;
    set_opaque_pointer(&value, &payload);

    circa::Value handle;
    move(&value, set_handle_value(&handle, my_release);


}

void register_tests()
{
    REGISTER_TEST_CASE(handle::test_simple);
}

}
