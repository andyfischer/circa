// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "handle.h"
#include "kernel.h"
#include "importing.h"
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

    test_assert(is_null(handle_get_value(&handle1)));
    test_assert(is_null(handle_get_value(&handle2)));

    set_int(handle_get_value(&handle1), 777);
    test_equals(handle_get_value(&handle1), "777");
    test_equals(handle_get_value(&handle2), "777");

    set_null(&handle1);

    test_equals(handle_get_value(&handle2), "777");
}

int gTimesReleaseCalled = 0;

void my_release_func(caStack* stack)
{
    gTimesReleaseCalled++;
}

void test_release()
{
    Branch branch;
    branch.compile("type T = handle_type()");
    branch.compile("def T.release(self)");

    install_function(&branch, "T.release", my_release_func);

    Type* T = find_type(&branch, "T");
    test_assert(T != NULL);
    test_assert(find_method(NULL, T, "release") != NULL);

    gTimesReleaseCalled = 0;

    Value value;
    make(T, &value);
    test_assert(is_handle(&value));
    test_equals(gTimesReleaseCalled, 0);

    set_int(handle_get_value(&value), 5);
    set_null(&value);

    test_equals(gTimesReleaseCalled, 1);

    gTimesReleaseCalled = 0;

    Value value2;
    make(T, &value);
    copy(&value, &value2);

    test_assert(gTimesReleaseCalled == 0);

    set_null(&value2);
    test_assert(gTimesReleaseCalled == 0);

    set_null(&value);
    test_assert(gTimesReleaseCalled == 1);

    gTimesReleaseCalled = 0;
    make(T, &value);
    make(T, &value2);

    set_null(&value);
    test_assert(gTimesReleaseCalled == 1);

    set_null(&value2);
    test_assert(gTimesReleaseCalled == 2);
}

void register_tests()
{
    REGISTER_TEST_CASE(handle::test_value_is_shared);
    REGISTER_TEST_CASE(handle::test_release);
}

}
