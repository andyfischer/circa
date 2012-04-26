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

World* alloc_world()
{
    World* world = (World*) malloc(sizeof(*world));

    initialize_null(&world->actorList);
    set_list(&world->actorList, 0);

    world->mainStack = circa_alloc_stack(world);

    return world;
}

caValue* find_actor(World* world, const char* name)
{
    caValue* actors = &world->actorList;
    for (int i=0; i < list_length(actors); i++) {
        caValue* actor = list_get(actors, i);
        if (string_eq(list_get(actor, 0), name))
            return actor;
    }
    return NULL;
}

void actor_post_message(caValue* actor, caValue* message)
{
    caValue* queue = list_get(actor, 2);

    copy(message, list_append(queue));
}

void actor_run_message(caStack* stack, caValue* actor, caValue* message)
{
    Branch* branch = as_branch(list_get(actor, 1));
    refresh_script(branch);

    push_frame(stack, branch);
    copy(message, circa_input(stack, 0));

    // TODO: state

    run_interpreter(stack);
}

void actor_run_queue(caWorld* world, caValue* actor, int maxMessages)
{
    caValue* messages = list_get(actor, 2);
    caStack* stack = world->mainStack;

    // Iterate once for each message
    // Note that new messages might be appended to the queue while we are running;
    // don't run the new messages right now.

    int count = circa_count(messages);
    if (count == 0)
        return;

    // Enforce maximum messages per call
    if (maxMessages > 0 && count >= maxMessages)
        count = maxMessages;

    for (int i=0; i < count; i++) {
        caValue* message = list_get(messages, i);
        actor_run_message(stack, actor, message);
    }

    // Remove the messages that we handled
    Value newQueue;
    list_slice(messages, count, -1, &newQueue);
    swap(&newQueue, messages);
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

    actor_post_message(actor, message);
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


void circa_actor_run_queue(caWorld* world, const char* actorName, int maxMessages)
{
    caValue* actor = find_actor(world, actorName);

    if (actor == NULL) {
        printf("couldn't find actor named: %s\n", actorName);
        return;
    }

    actor_run_queue(world, actor, maxMessages);
}

void circa_actor_run_all_queues(caWorld* world, int maxMessages)
{
    for (int i=0; i < list_length(&world->actorList); i++) {
        actor_run_queue(world, list_get(&world->actorList, i), maxMessages);
    }
}
