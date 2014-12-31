// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "interpreter.h"
#include "kernel.h"
#include "modules.h"
#include "native_patch.h"
#include "world.h"

namespace native_patch_test {
#if 0

void my_add(Stack* stack)
{
    circa_set_int(circa_output(stack, 0),
            circa_int_input(stack, 0) + circa_int_input(stack, 1));
}

void my_5(Stack* stack)
{
    circa_set_int(circa_output(stack, 0), 5);
}

void my_6(Stack* stack)
{
    circa_set_int(circa_output(stack, 0), 6);
}

void patch_manually()
{
    World* world = global_world();

    // Run with an unpatched 'my_add'
    Block block;
    block.compile("def my_add(int a, int b) -> int { a + a }");
    block.compile("test_spy(my_add(1 2))");

    Stack stack;
    stack_init(&stack, &block);
    test_spy_clear();
    circa_run(&stack);

    test_equals(test_spy_get_results(), "[2]");

    // Create a patch for my_add
    NativePatch* patch = insert_native_patch(world, "patch_manually");
    module_patch_function(patch, "my_add", my_add);
    //native_patch_apply_patch(patch, &block);

    stack_init(&stack, &block);
    test_spy_clear();
    circa_run(&stack);

    test_equals(test_spy_get_results(), "[3]");

    remove_native_patch(world, "patch_manually");
}

void trigger_change()
{
    World* world = global_world();

    // Don't patch manually, add a change action and trigger it.
    Block* block = fetch_module(world, "trigger_change_test");
    block->compile("def f() -> int { 1 }");
    block->compile("test_spy(f())");

    NativePatch* patch = insert_native_patch(world, "trigger_change_test");

    Stack stack;
    stack_init(&stack, block);
    test_spy_clear();
    circa_run(&stack);

    // First pass, patch not in effect.
    test_equals(test_spy_get_results(), "[1]");

    // Now, patch in effect.
    module_patch_function(patch, "f", my_5);
    native_patch_finish_change(patch);

    stack_init(&stack, block);
    test_spy_clear();
    circa_run(&stack);

    test_equals(test_spy_get_results(), "[5]");

    remove_native_patch(world, "trigger_change_test");
}

void new_function_patched_by_world()
{
    World* world = global_world();

    // First create the patch, as part of the global world.
    NativePatch* patch = insert_native_patch(global_world(), "nativemod_block");
    module_patch_function(patch, "my_add", my_add);
    native_patch_finish_change(patch);

    // Now create our function, it should get patched instantly.
    Block* block = fetch_module(global_world(), "nativemod_block");
    block->compile("def my_add(int a, int b) -> int { a + a }");
    block->compile("test_spy(my_add(1 2))");

    Stack stack;
    stack_init(&stack, block);
    test_spy_clear();
    circa_run(&stack);

    test_equals(test_spy_get_results(), "[3]");

    remove_native_patch(world, "trigger_change_test");
}

void patch_manually_public_api()
{
    test_write_fake_file("Module.ca", 1, "def my_5() -> int; my_5() -> output");

    caWorld* world = global_world();
    caBlock* module = circa_load_module_from_file(world, "Module", "Module.ca");

    caNativePatch* npatch = circa_create_native_patch(world, "Module");
    circa_patch_function(npatch, "my_5", my_5);
    circa_finish_native_patch(npatch);

    Stack* stack = circa_create_stack(world);
    circa_push_module(stack, "Module");
    circa_run(stack);

    test_assert(stack);
    test_equals(circa_output(stack, 0), "5");

    circa_free_stack(stack);
}
#endif

void register_tests()
{
#if 0
    REGISTER_TEST_CASE(native_patch_test::patch_manually);
    REGISTER_TEST_CASE(native_patch_test::trigger_change);
    REGISTER_TEST_CASE(native_patch_test::new_function_patched_by_world);
    REGISTER_TEST_CASE(native_patch_test::patch_manually_public_api);
#endif
}

} // namespace native_patch_test
