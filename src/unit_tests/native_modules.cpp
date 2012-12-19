// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "evaluation.h"
#include "fakefs.h"
#include "kernel.h"
#include "modules.h"
#include "native_modules.h"
#include "world.h"

namespace native_modules {

void my_add(caStack* stack)
{
    circa_set_int(circa_output(stack, 0),
            circa_int_input(stack, 0) + circa_int_input(stack, 1));
}

void my_5(caStack* stack)
{
    circa_set_int(circa_output(stack, 0), 5);
}

void my_6(caStack* stack)
{
    circa_set_int(circa_output(stack, 0), 6);
}

void patch_manually()
{
    // Run with an unpatched 'my_add'
    Block block;
    block.compile("def my_add(int a, int b) -> int { a + a }");
    block.compile("namespace ns { def my_add(int a, int b) -> int { a + a } }");
    block.compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, &block);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[2]");

    // Create a patch for my_add
    NativePatch* module = create_native_module(global_world());
    module_patch_function(module, "my_add", my_add);
    native_module_apply_patch(module, &block);

    reset_stack(&stack);
    push_frame(&stack, &block);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[3]");

    free_native_module(module);
}

void patch_manually_ns()
{
    // Similar to 'patch_manually' test, but with a namespaced name.
    Block block;
    block.compile("def f1() -> int { 1 }");
    block.compile("namespace ns_a { def f1() -> int { 1 } }");
    block.compile("namespace ns_b { namespace ns_a { def f1() -> int { 1 } } }");
    block.compile("test_spy(f1())");
    block.compile("test_spy(ns_a:f1())");
    block.compile("test_spy(ns_b:ns_a:f1())");

    NativePatch* module = create_native_module(global_world());
    module_patch_function(module, "ns_a:f1", my_5);
    module_patch_function(module, "ns_b:ns_a:f1", my_6);
    native_module_apply_patch(module, &block);

    Stack stack;
    push_frame(&stack, &block);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1, 5, 6]");

    free_native_module(module);
}

void trigger_change()
{
    // Don't patch manually, add a change action and trigger it.
    Block* block = fetch_module(global_world(), "trigger_change_test");
    block->compile("def f() -> int { 1 }");
    block->compile("test_spy(f())");

    NativePatch* module = add_native_patch(global_world(), "trigger_change_test");

    Stack stack;
    push_frame(&stack, block);
    test_spy_clear();
    run_interpreter(&stack);

    // First pass, patch not in effect.
    test_equals(test_spy_get_results(), "[1]");

    // Now, patch in effect.
    module_patch_function(module, "f", my_5);
    native_module_finish_change(module);

    reset_stack(&stack);
    push_frame(&stack, block);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[5]");

    free_native_module(module);
}

void new_function_patched_by_world()
{
    // First create the module, as part of the global world.
    NativePatch* module = add_native_patch(global_world(), "nativemod_block");
    module_patch_function(module, "my_add", my_add);
    native_module_finish_change(module);

    // Now create our function, it should get patched instantly.
    Block* block = fetch_module(global_world(), "nativemod_block");
    block->compile("def my_add(int a, int b) -> int { a + a }");
    block->compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, block);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[3]");

    delete_native_module(global_world(), "native_modules_test");
}

void patch_manually_public_api()
{
    FakeFilesystem fs;
    fs.set("Module.ca", "def my_5() -> int; result = my_5()");

    caWorld* world = global_world();
    caBlock* module = circa_load_module_from_file(world, "Module", "Module.ca");

    caNativePatch* npatch = circa_create_native_patch(world, "Module");
    circa_patch_function(npatch, "my_5", my_5);
    circa_finish_native_module(npatch);

    caStack* stack = circa_alloc_stack(world);
    circa_push_module(stack, "Module");
    circa_run(stack);

    test_assert(stack);
    test_equals(circa_output(stack, 0), "5");

    circa_dealloc_stack(stack);
}

void register_tests()
{
    REGISTER_TEST_CASE(native_modules::patch_manually);
    REGISTER_TEST_CASE(native_modules::patch_manually_ns);
    REGISTER_TEST_CASE(native_modules::trigger_change);
    REGISTER_TEST_CASE(native_modules::new_function_patched_by_world);
    REGISTER_TEST_CASE(native_modules::patch_manually_public_api);
}

}
