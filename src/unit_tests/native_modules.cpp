// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "unit_test_common.h"

#include "evaluation.h"
#include "kernel.h"
#include "native_modules.h"

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

    test_spy_clear();
    Stack stack;
    push_frame(&stack, &branch);
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[2]");

    // Create a patch for my_add
    NativeModule* module = create_native_module();
    module_patch_function(module, "my_add", my_add);
    module_manually_patch_branch(module, &branch);

    test_spy_clear();
    reset_stack(&stack);
    push_frame(&stack, &branch);
    run_interpreter(&stack);

    test_equals(test_spy_get_results(), "[3]");

    free_native_module(module);
}

void register_tests()
{
    REGISTER_TEST_CASE(native_modules::patch_manually);
}

}
