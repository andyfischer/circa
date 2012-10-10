// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "evaluation.h"
#include "kernel.h"
#include "native_modules.h"
#include "world.h"

namespace native_modules {

void my_add(caStack* stack)
{
    circa_set_int(circa_output(stack, 0),
            circa_int_input(stack, 0) + circa_int_input(stack, 1));
}

void patch_manually()
{
    // Run with an unpatched 'my_add'
    Branch branch;
    branch.compile("def my_add(int a, int b) -> int { a + a }");
    branch.compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[2]");

    // Create a patch for my_add
    NativeModule* module = create_native_module();
    module_patch_function(module, "my_add", my_add);
    module_manually_patch_branch(module, &branch);

    reset_stack(&stack);
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[3]");

    free_native_module(module);
}

void new_function_patched_by_world()
{
    // First create the module, as part of the global world.
    NativeModule* module = add_native_module(global_world(), name_from_string("test_module_34"));
    module_patch_function(module, "my_add", my_add);

    // Now create our function, it should be patched.
    Branch branch;
    branch.compile("def my_add(int a, int b) -> int { a + a }");
    branch.compile("test_spy(my_add(1 2))");

    Stack stack;
    push_frame(&stack, &branch);
    test_spy_clear();
    run_interpreter(&stack);

    // test_equals(test_spy_get_results(), "[3]");
}

void register_tests()
{
    REGISTER_TEST_CASE(native_modules::patch_manually);
    REGISTER_TEST_CASE(native_modules::new_function_patched_by_world);
}

}
