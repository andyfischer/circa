// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "evaluation.h"
#include "object.h"
#include "type.h"

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

    Block block;
    block.compile(
        "type MyType; \n"
        "def create_object() -> MyType\n"
        "def check_object(MyType t)\n"
        "s = create_object()\n"
        "check_object(s)\n"
            );

    circa_install_function(&block, "create_object", create_object);
    circa_install_function(&block, "check_object", check_object);

    circa_setup_object_type(circa_find_type_local(&block, "MyType"),
            sizeof(CustomObject), CustomObjectRelease);

    // Shouldn't allocate any objects before running.
    test_equals(g_currentlyAllocated, 0);
    test_equals(g_totalAllocated, 0);

    Stack stack;
    push_frame(&stack, &block);
    run_interpreter(&stack);
    test_assert(&stack);
    circa_clear_stack(&stack);

    // Running the script should only cause 1 object allocation.
    test_equals(g_currentlyAllocated, 0);
    test_equals(g_totalAllocated, 1);
}


void test_type_not_prematurely_used()
{
    // Verify that a circa-defined type is not used until interpreter time. Modifying
    // a type's release() handler after there are already instances of it, is not good.
    
    Block block;
    block.compile(
        "type MyType; \n"
        "def f() -> MyType\n"
        "def g(MyType t)\n"
        "s = f()\n"
        "g(s)\n"
        "l = [s s s]\n"
        "type MyCompoundType {\n"
        "  MyType t\n"
        "}\n"
        "state MyType st\n"
        "state MyType st2 = make(MyType)\n"
        );

    Type* myType = (Type*) circa_find_type_local(&block, "MyType");
    Type* myCompoundType = (Type*) circa_find_type_local(&block, "MyCompoundType");
    test_assert(!myType->inUse);
    test_assert(!myCompoundType->inUse);

    circa::Value value1, value2;
    make(myType, &value1);
    make(myCompoundType, &value2);

    test_assert(myType->inUse);
    test_assert(myCompoundType->inUse);
}

void register_tests()
{
    REGISTER_TEST_CASE(c_objects::test_custom_object);
    REGISTER_TEST_CASE(c_objects::test_type_not_prematurely_used);
}

} // namespace c_objects
