
#include "circa/circa.h"

int main(int argc, char** argv)
{
    caWorld* world = circa_initialize();

    circa_actor_new_from_file(world, "ActorA", "ActorA.ca");
    circa_actor_new_from_file(world, "ActorB", "ActorB.ca");

    caValue* msg = circa_alloc_value();
    circa_set_int(msg, 0);

    circa_actor_post_message(world, "ActorA", msg);

    for (int i=0; i < 10; i++)
        circa_actor_run_all_queues(world);

    return 0;
}
