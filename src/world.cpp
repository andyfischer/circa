// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "branch.h"
#include "evaluation.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
#include "string_type.h"
#include "tagged_value.h"
#include "world.h"

namespace circa {

caWorld* alloc_world()
{
    caWorld* world = (caWorld*) malloc(sizeof(world));

    initialize_null(&world->actorList);
    set_list(&world->actorList, 0);

    world->mainStack = circa_alloc_stack(world);

    return world;
}

caValue* find_actor(caWorld* world, const char* name)
{
    caValue* actors = &world->actorList;
    for (int i=0; i < list_length(actors); i++) {
        caValue* actor = list_get(actors, i);
        if (string_eq(list_get(actor, 0), name))
            return actor;
    }
    return NULL;
}

void actor_run_message(caStack* stack, caValue* actor, caValue* message)
{
    Branch* branch = as_branch(list_get(actor, 1));
    refresh_script(branch);

    push_frame((Stack*) stack, branch);
    copy(message, circa_input(stack, 0));

    // TODO: state
}

} // namespace circa

using namespace circa;

void circa_actor_new_from_file(caWorld* world, const char* actorName, const char* filename)
{
    Branch* module = load_module_from_file(actorName, filename);

    caValue* actor = list_append(&world->actorList);
    create(TYPES.actor, actor);

    // Actor has shape:
    // { String name, Branch branch, List incomingQueue, any stateVal }
    set_string(list_get(actor, 0), actorName);
    set_branch(list_get(actor, 1), module);
}


void circa_actor_post_message(caWorld* world, const char* actorName, caValue* message)
{
    caValue* actor = find_actor(world, actorName);
    if (actor == NULL) {
        printf("couldn't find actor named: %s\n", actorName);
        return;
    }

    caValue* queue = list_get(actor, 2);

    copy(message, list_append(queue));
}

void circa_actor_run_message(caWorld* world, const char* actorName, caValue* message)
{
    caValue* actor = find_actor(world, actorName);
    if (actor == NULL) {
        printf("couldn't find actor named: %s\n", actorName);
        return;
    }
    caStack* stack = world->mainStack;
    actor_run_message(stack, actor, message);
}

static void circa_actor_run_queue_internal(caWorld* world, caValue* actor)
{
    caValue* messages = list_get(actor, 2);
    caStack* stack = world->mainStack;

    // Iterate once for each message
    // Note that new messages might be appended to the queue while we are running;
    // don't run the new messages right now.

    int count = circa_count(messages);
    if (count == 0)
        return;

    for (int i=0; i < count; i++) {
        caValue* message = list_get(messages, i);
        actor_run_message(stack, actor, message);
    }

    // Remove the messages that we handled
    Value newQueue;
    list_slice(messages, count, -1, &newQueue);
    swap(&newQueue, messages);
}

void circa_actor_run_queue(caWorld* world, const char* actorName)
{
    caValue* actor = find_actor(world, actorName);

    if (actor == NULL) {
        printf("couldn't find actor named: %s\n", actorName);
        return;
    }

    circa_actor_run_queue_internal(world, actor);
}

void circa_actor_run_all_queues(caWorld* world)
{
    for (int i=0; i < list_length(&world->actorList); i++) {
        circa_actor_run_queue_internal(world, list_get(&world->actorList, i));
    }
}
