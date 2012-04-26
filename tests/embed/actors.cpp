
#include "circa/circa.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_actor_new_from_file(world, "ActorA", "tests/embed/ActorA.ca");
    circa_actor_new_from_file(world, "ActorB", "tests/embed/ActorB.ca");

    caValue* msg = circa_alloc_value();
    circa_set_int(msg, 0);

    circa_actor_post_message(world, "ActorA", msg);

    for (int i=0; i < 10; i++)
        circa_actor_run_all_queues(world, 10);

    return 0;
}
