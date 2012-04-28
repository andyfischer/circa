// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include <cstdio>

#include "circa/circa.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_add_module_search_path(world, "tests/embed");

    //circa_actor_new_from_file(world, "ActorA", "tests/embed/ActorA.ca");
    //circa_actor_new_from_file(world, "ActorB", "tests/embed/ActorB.ca");

    caValue* msg = circa_alloc_value();
    circa_set_int(msg, 0);

    circa_actor_post_message(world, "ActorA", msg);

    for (int i=0; i < 10; i++)
        circa_actor_run_all_queues(world, 10);

    printf("Resetting and starting actors C and D...\n");
    circa_actor_clear_all(world);

    //circa_actor_new_from_file(world, "ActorC", "tests/embed/ActorC.ca");
    //circa_actor_new_from_file(world, "ActorD", "tests/embed/ActorD.ca");

    circa_actor_post_message(world, "ActorC", msg);

    for (int i=0; i < 10; i++)
        circa_actor_run_all_queues(world, 10);

    return 0;
}
