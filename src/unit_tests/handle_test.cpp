// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "kernel.h"
#include "importing.h"
#include "interpreter.h"
#include "list.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "stack.h"
#include "tagged_value.h"

namespace handle_test {

struct MyValue {
    int magic_number;
    bool has_been_checked;
    bool has_been_freed;
};

MyValue* gMyValue;

void new_value(caStack* stack)
{
    test_assert(gMyValue == NULL);

    gMyValue = new MyValue();
    gMyValue->magic_number = 12345;
    gMyValue->has_been_checked = false;
    gMyValue->has_been_freed = false;

    caValue* wrapper = circa_create_default_output(stack, 0);
    circa_set_raw_pointer(circa_index(wrapper, 0), gMyValue);
}

void check_value(caStack* stack)
{
    MyValue* value = (MyValue*) circa_raw_pointer(circa_index(circa_input(stack, 0), 0));
    test_assert(value->magic_number == 12345);
    test_assert(!value->has_been_checked);
    test_assert(!value->has_been_freed);

    value->has_been_checked = true;
}

void release_value(caValue* v)
{
    MyValue* value = (MyValue*) circa_raw_pointer(v);

    test_assert(value->magic_number == 12345);
    test_assert(value->has_been_checked);
    test_assert(!value->has_been_freed);

    value->has_been_freed = true;
}

void test_handle_pattern()
{
    NativePatch* patch = insert_native_patch(global_world(), "handle_test_module");
    module_patch_function(patch, "new_value", new_value);
    module_patch_function(patch, "check_value", check_value);
    module_patch_type_release(patch, "ValueHandle", release_value);
    native_patch_finish_change(patch);

    Block* block = fetch_module(global_world(), "handle_test_module");
    block->compile("type ValueHandle :nocopy ;");
    block->compile("type ValueWrapper { ValueHandle handle }");
    block->compile("def new_value() -> ValueWrapper");
    block->compile("def check_value(ValueWrapper)");
    block->compile("v = new_value()");
    block->compile("check_value(v)");

    Stack stack;
    stack_init(&stack, block);
    stack_run(&stack);
    stack_init(&stack, block);

    test_assert(gMyValue != NULL);
    test_assert(gMyValue->magic_number == 12345);
    test_assert(gMyValue->has_been_checked);
    test_assert(gMyValue->has_been_freed);
}

void register_tests()
{
    REGISTER_TEST_CASE(handle_test::test_handle_pattern);
}

} // namespace handle_test
