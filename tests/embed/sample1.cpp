
#include "circa/circa.h"

#include "../../src/block.h"
#include "../../src/kernel.h"
#include "../../src/names.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_load_module_from_file(world, "ClassA", "ClassA.ca");

    // circa_dump_b(circa_kernel(world));

    caStack* stack = circa_alloc_stack(world);

    circa_push_function_by_name(stack, "create_ClassA");
    circa_run(stack);

    if (circa_has_error(stack))
        circa_print_error_to_stdout(stack);

    caValue* classA = circa_alloc_value();
    circa_move(circa_output(stack, 0), classA);
    circa_pop(stack);

    // Dump to stdout
    circa_push_function_by_name(stack, "ClassA.dump");
    circa_copy(classA, circa_input(stack, 0));
    circa_run(stack);
    if (circa_has_error(stack))
        circa_print_error_to_stdout(stack);
    circa_pop(stack);

    for (int i=0; i < 5; i++) {
        // Increment
        circa_push_function_by_name(stack, "ClassA.increment");
        circa_copy(classA, circa_input(stack, 0));
        circa_run(stack);
        if (circa_has_error(stack))
            circa_print_error_to_stdout(stack);

        // Using index #1 not 0:
        circa_move(circa_output(stack, 1), classA);
        circa_pop(stack);

        // And dump
        circa_push_function_by_name(stack, "ClassA.dump");
        circa_copy(classA, circa_input(stack, 0));
        circa_run(stack);
        if (circa_has_error(stack))
            circa_print_error_to_stdout(stack);
        circa_pop(stack);
    }

    circa_dealloc_value(classA);
    circa_dealloc_stack(stack);
    circa_shutdown(world);
}
