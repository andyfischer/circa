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

int gTimesReleaseCalled = 0;

void my_release_func(caValue* value)
{
    gTimesReleaseCalled++;
}

void test_release()
{
    Type type;
    setup_handle_type(&type);
    handle_type_set_release_func(&type, my_release_func);

    gTimesReleaseCalled = 0;

    Value handle1;
    make(&type, &handle1);

    set_int(get_handle_value(&handle1), 888);

    test_assert(gTimesReleaseCalled == 0);

    set_null(&handle1);

    test_assert(gTimesReleaseCalled == 1);

    gTimesReleaseCalled = 0;

    Value handle2;
    make(&type, &handle1);
    copy(&handle1, &handle2);

    test_assert(gTimesReleaseCalled == 0);

    set_null(&handle2);
    test_assert(gTimesReleaseCalled == 0);

    set_null(&handle1);
    test_assert(gTimesReleaseCalled == 1);

    gTimesReleaseCalled = 0;
    make(&type, &handle1);
    make(&type, &handle2);

    set_null(&handle1);
    test_assert(gTimesReleaseCalled == 1);

    set_null(&handle2);
    test_assert(gTimesReleaseCalled == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(handle::test_value_is_shared);
    REGISTER_TEST_CASE(handle::test_release);
}

}
