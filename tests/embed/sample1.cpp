
#include "circa/circa.h"

#include "../../src/branch.h"
#include "../../src/kernel.h"
#include "../../src/names.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_load_module_from_file(world, "ClassA", "ClassA.ca");

    // circa_dump_b(circa_kernel(world));

    caStack* stack = circa_alloc_stack(world);

    circa_push_function(stack, "create_ClassA");
    circa_run(stack);

    if (circa_has_error(stack))
        circa_print_error_to_stdout(stack);

    caValue* classA = circa_alloc_value();
    circa_move(circa_frame_output(stack, 0), classA);
    circa_pop(stack);

    circa_push_function(stack, "ClassA.dump");
    circa_copy(classA, circa_frame_input(stack, 0));
    circa_run(stack);
    if (circa_has_error(stack))
        circa_print_error_to_stdout(stack);
    circa_pop(stack);

    // Increment
    for (int i=0; i < 5; i++) {
        circa_push_function(stack, "ClassA.increment");
        circa_copy(classA, circa_frame_input(stack, 0));
        circa_run(stack);
        if (circa_has_error(stack))
            circa_print_error_to_stdout(stack);

        // Using index #1 not 0:
        circa_move(circa_frame_output(stack, 1), classA);
        circa_pop(stack);

        // And dump
        circa_push_function(stack, "ClassA.dump");
        circa_copy(classA, circa_frame_input(stack, 0));
        circa_run(stack);
        if (circa_has_error(stack))
            circa_print_error_to_stdout(stack);
        circa_pop(stack);
    }

    //circa_call_method

    circa_dealloc_value(classA);
    circa_dealloc_stack(stack);
    circa_shutdown(world);
}
