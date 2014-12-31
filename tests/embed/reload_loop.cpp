// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "circa/circa.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_add_module_search_path(world, "tests/embed");

    int iteration = 0;

    while (true) {

        Value* value = circa_alloc_value();
        circa_set_int(value, iteration);

        circa_actor_run_message(world, "TestA", value);
        sleep(1);
        iteration++;
    }

    circa_shutdown(world);
}
