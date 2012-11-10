// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "handle.h"
#include "tagged_value.h"
#include "type.h"

namespace handle {

void test_value_is_shared()
{
    Type type;
    setup_handle_type(&type);

    Value handle1;
    Value handle2;

    make(&type, &handle1);
    copy(&handle1, &handle2);

    test_assert(is_null(get_handle_value(&handle1)));
    test_assert(is_null(get_handle_value(&handle2)));

    set_int(get_handle_value(&handle1), 777);
    test_equals(get_handle_value(&handle1), "777");
    test_equals(get_handle_value(&handle2), "777");

    set_null(&handle1);

    test_equals(get_handle_value(&handle2), "777");
}

/*
void my_release(caValue* value)
{
    test_assert(gSlot[as_int(value)]);
    gSlot[as_int(value)] = false;
}
*/

void register_tests()
{
    REGISTER_TEST_CASE(handle::test_value_is_shared);
}

}
