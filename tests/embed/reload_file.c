

#include <unistd.h>
#include <stdio.h> 

#include "circa/circa.h"

// This script will constantly write a script to a file, and will constantly
// refresh and run that module, to make sure that block reloading works correctly.

void write_script_to_file(int key)
{
    FILE* file = fopen("file_to_reload.ca", "w");
    fprintf(file, "def f() -> int\n");
    fprintf(file, "  return %d\n", key);
    fclose(file);
}

int main(int argc, char** argv)
{
    const int iteratations = 100;

    caWorld* world = circa_initialize();
    Stack* stack = circa_create_stack(world);

    // Write initial version
    write_script_to_file(0);

    caBlock* module = circa_load_module_from_file(world, "file_to_reload", "file_to_reload.ca");

    int currentKey = 0;

    for (int i=0; i < iteratations; i++) {

        // Update file on every other iteration
        if ((i % 2) == 1) {
            printf("writing to file: %d\n", i);
            currentKey = i;
            write_script_to_file(i);
            sleep(2);
        }

        circa_refresh_module(module);

        circa_push_function_by_name(stack, "f");
        circa_run(stack);
        if (circa_has_error(stack)) {
            circa_print_error_to_stdout(stack);
            break;
        }

        int readValue = circa_int(circa_output(stack, 0));

        if (currentKey != readValue) {
            printf("Failed, currentKey (%d) != readValue (%d)\n", currentKey, readValue);
            break;
        }
        circa_pop(stack);
    }

    circa_shutdown(world);
}
