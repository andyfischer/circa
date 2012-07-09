// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "framework.h"
#include "object.h"

#include "evaluation.h"

using namespace circa;

namespace c_objects {

struct CustomObject
{
    bool initialized;
    bool checked;
    bool destroyed;
};

int g_currentlyAllocated = 0;
int g_totalAllocated = 0;

void create_object(caStack* stack)
{
    CustomObject* object = (CustomObject*) circa_create_object_output(stack, 0);

    object->initialized = true;
    object->checked = false;
    object->destroyed = false;

    g_currentlyAllocated++;
    g_totalAllocated++;
}

void check_object(caStack* stack)
{
    CustomObject* object = (CustomObject*) circa_object_input(stack, 0);

    test_assert(object->initialized);
    test_assert(!object->checked);
    test_assert(!object->destroyed);

    object->checked = true;
}

void CustomObjectRelease(void* object)
{
    CustomObject* obj = (CustomObject*) object;
    test_assert(obj->initialized);
    test_assert(obj->checked);
    test_assert(!obj->destroyed);

    g_currentlyAllocated--;
}

void test_custom_object()
{
    g_currentlyAllocated = 0;
    g_totalAllocated = 0;

    Branch branch;
    branch.compile(
        "type MyType; \n"
        "def create_object() -> MyType\n"
        "def check_object(MyType t)\n"
        "s = create_object()\n"
        "check_object(s)\n"
            );

    circa_install_function(&branch, "create_object", create_object);
    circa_install_function(&branch, "check_object", check_object);

    circa_setup_object_type(circa_find_type(&branch, "MyType"),
            sizeof(CustomObject), CustomObjectRelease);

    // Shouldn't allocate any objects before running.
    test_equals(g_currentlyAllocated, 0);
    test_equals(g_totalAllocated, 0);

    Stack stack;
    push_frame(&stack, &branch);
    run_interpreter(&stack);
    test_assert(&stack);
    circa_clear_stack(&stack);

    // Running the script should only cause 1 object allocation.
    test_equals(g_currentlyAllocated, 0);
    test_equals(g_totalAllocated, 1);
}

} // namespace c_objects

void c_objects_register_tests()
{
    REGISTER_TEST_CASE(c_objects::test_custom_object);
}
