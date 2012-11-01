// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "evaluation.h"
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
    Branch branch;
    branch.compile("def my_add(int a, int b) -> int { a + a }");
    branch.compile("namespace ns { def my_add(int a, int b) -> int { a + a } }");
    branch.compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[2]");

    // Create a patch for my_add
    NativeModule* module = create_native_module(global_world());
    module_patch_function(module, "my_add", my_add);
    native_module_apply_patch(module, &branch);

    reset_stack(&stack);
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[3]");

    free_native_module(module);
}

void patch_manually_ns()
{
    // Similar to 'patch_manually' test, but with a namespaced name.
    Branch branch;
    branch.compile("def f1() -> int { 1 }");
    branch.compile("namespace ns_a { def f1() -> int { 1 } }");
    branch.compile("namespace ns_b { namespace ns_a { def f1() -> int { 1 } } }");
    branch.compile("test_spy(f1())");
    branch.compile("test_spy(ns_a:f1())");
    branch.compile("test_spy(ns_b:ns_a:f1())");

    NativeModule* module = create_native_module(global_world());
    module_patch_function(module, "ns_a:f1", my_5);
    module_patch_function(module, "ns_b:ns_a:f1", my_6);
    native_module_apply_patch(module, &branch);

    Stack stack;
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[1, 5, 6]");

    free_native_module(module);
}

void trigger_change()
{
    // Don't patch manually, add a change action and trigger it.
    Branch* branch = add_module(global_world(), "trigger_change_test");
    branch->compile("def f() -> int { 1 }");
    branch->compile("test_spy(f())");

    NativeModule* module = create_native_module(global_world());

    module_patch_function(module, "f", my_5);
    native_module_add_change_action_patch_branch(module, "trigger_change_test");

    Stack stack;
    push_frame(&stack, branch);
    test_spy_clear();
    run_interpreter(&stack);

    // First pass, patch not in effect.
    test_equals(test_spy_get_results(), "[1]");

    // Now, patch in effect.
    native_module_on_change(module);

    reset_stack(&stack);
    push_frame(&stack, branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[5]");

    free_native_module(module);
}

void new_function_patched_by_world()
{
    // First create the module, as part of the global world.
    NativeModule* module = add_native_module(global_world(), "native_modules_test");
    module_patch_function(module, "my_add", my_add);

    // Now create our function, it should get patched instantly.
    Branch branch;
    branch.compile("def my_add(int a, int b) -> int { a + a }");
    branch.compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    // FIXME
    // test_equals(test_spy_get_results(), "[3]");

    delete_native_module(global_world(), "native_modules_test");
}

void register_tests()
{
    REGISTER_TEST_CASE(native_modules::patch_manually);
    REGISTER_TEST_CASE(native_modules::patch_manually_ns);
    REGISTER_TEST_CASE(native_modules::new_function_patched_by_world);
    REGISTER_TEST_CASE(native_modules::trigger_change);
}

}
