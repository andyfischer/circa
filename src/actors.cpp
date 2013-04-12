// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "actors.h"
#include "building.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"
#include "world.h"


namespace circa {

// Actor system v3
//
// In this scheme, an "actor" is a Block with associated state. Callers can share an
// actor (and thus share its state, indirectly). Callers can 'call' the actor to use
// it. This calling mechanism is syncronous, there's no direct support for async.
// (But callers can simulate async by injecting into queues).

#if 0
Stack* create_actor(World* world, Block* block)
{
    ca_assert(block != NULL);
    Stack* stack = create_stack(world);
    push_frame(stack, block);
    return stack;
}
#endif

bool state_inject(Stack* stack, caValue* name, caValue* value)
{
    caValue* state = stack_get_state(stack);
    Block* block = top_frame(stack)->block;

    // Initialize stateValue if it's currently null.
    if (is_null(state))
        make(block->stateType, state);

    caValue* slot = get_field(state, name, NULL);
    if (slot == NULL)
        return false;

    touch(state);
    copy(value, get_field(state, name, NULL));
    return true;
}

void context_inject(Stack* stack, caValue* name, caValue* value)
{
    Frame* frame = top_frame(stack);

    if (is_null(&frame->dynamicScope))
        set_hashtable(&frame->dynamicScope);

    copy(value, hashtable_insert(&frame->dynamicScope, name));
}

#if 0
void actor_run(Stack* actor)
{
    Stack* stack = actor->stack;
    ca_assert(top_frame_parent(stack) == NULL);

    stack_restart(stack);
    run_interpreter(stack);

    if (error_occurred(stack)) {
        printf("Error occurred in actor_run\n");
        dump(stack);
    }
}
#endif

#if 0

// Actor system v2:

struct ActorSpace {
    int id;

    int currentInbox;
    int nextInbox;

    Value actorList;

    // Maps names to address values.
    Value names;

    // Addresses of special actors.
    Value moduleLoader;
    Value errorListener;

    World* world;
};

struct Actor {
    ActorSpace* space;
    Block* block;
    Stack* stack;
    Value address;
    Value label;

    Value inbox[2];
    int nextIndex;
};

void actor_run(Actor* actor);

ActorSpace* create_actor_space(World* world)
{
    ActorSpace* space = new ActorSpace();
    space->id = world->nextActorSpaceID++;
    space->currentInbox = 0;
    space->nextInbox = 1;
    set_list(&space->actorList, 0);
    set_hashtable(&space->names);
    space->world = world;
    return space;
}

void free_actor_space(ActorSpace* space)
{
    delete space;
}

Actor* create_actor(ActorSpace* space)
{
    Actor* actor = new Actor();
    actor->space = space;
    actor->block = NULL;
    actor->stack = create_stack(space->world);
    set_list(&actor->inbox[0], 0);
    set_list(&actor->inbox[1], 0);
    actor->nextIndex = 0;

    int index = list_length(&space->actorList);
    set_int(&actor->address, index);

    set_actor(list_append(&space->actorList), actor);

    return actor;
}

void free_actor(Actor* actor)
{
    delete actor;
}

void actor_bind_name(ActorSpace* space, Actor* actor, caValue* name)
{
    if (actor == NULL) {
        hashtable_remove(&space->names, name);
        return;
    }

    copy(&actor->address, hashtable_insert(&space->names, name));

    // Check if this is a specially recognized name, if so then give this actor a
    // special role.
    if (string_eq(name, "special:moduleLoader")) {
        set_value(&space->moduleLoader, &actor->address);
    } else if (string_eq(name, "special:errorListener")) {
        set_value(&space->errorListener, &actor->address);
    }
}

void actors_run_iteration(ActorSpace* space)
{
    // Debug code, verify that all current inboxes are empty. Otherwise these messages
    // will roll over to the 2nd iteration from now, which is surprising.
    // TODO consider disabling this in a release build.
    for (int i=0; i < list_length(&space->actorList); i++) {
        Actor* actor = as_actor(list_get(&space->actorList, i));
        caValue* current = &actor->inbox[space->currentInbox];
        if (!list_empty(current)) {
            internal_error("current inbox isn't empty in actors_run_iteration");
        }
    }
    
    space->currentInbox = space->nextInbox;
    space->nextInbox = space->nextInbox == 0 ? 1 : 0;

    for (int i=0; i < list_length(&space->actorList); i++) {
        Actor* actor = as_actor(list_get(&space->actorList, i));
        
        // No block, this actor should be handled manually.
        if (actor->block == NULL)
            continue;

        Value message;

        while (actor_consume_incoming(actor, &message)) {
            move(&message, actor_incoming_message_slot(actor));
            actor_run(actor);
        }
    }
}

void actor_run(Actor* actor)
{
    Stack* stack = actor->stack;
    ca_assert(top_block(stack) == actor->block);
    ca_assert(top_frame_parent(stack) == NULL);

    run_interpreter(stack);

    if (error_occurred(stack)) {
        // TODO: Send value to errorHandler (if any);
        printf("error occurred in actor_run\n");
    }

    stack_restart(stack);
}

void actor_set_block(Actor* actor, Block* block)
{
    actor->block = block;

    stack_reset(actor->stack);

    push_frame(actor->stack, block);
}

bool actor_has_incoming(Actor* actor)
{
    return ~list_empty(&actor->inbox[actor->space->currentInbox]);
}

bool actor_consume_incoming(Actor* actor, caValue* messageOut)
{
    caValue* inbox = &actor->inbox[actor->space->currentInbox];
    if (list_empty(inbox))
        return false;

    caValue* next = list_get(inbox, actor->nextIndex);
    move(next, messageOut);

    actor->nextIndex++;

    if (actor->nextIndex >= list_length(inbox)) {
        // Just finished the list.
        set_list(inbox, 0);
        actor->nextIndex = 0;
    }

    return true;
}

caValue* actor_incoming_message_slot(Actor* actor)
{
    ca_assert(top_frame_parent(actor->stack) == NULL);
    return frame_register(top_frame(actor->stack), 0);
}

caValue* actor_post(ActorSpace* space, caValue* address)
{
    Actor* actor = as_actor(list_get(&space->actorList, as_int(address)));
    caValue* inbox = &actor->inbox[actor->space->nextInbox];
    return list_append(inbox);
}

caValue* actor_post(Actor* actor)
{
    caValue* inbox = &actor->inbox[actor->space->nextInbox];
    return list_append(inbox);
}

caValue* actor_address(Actor* actor)
{
    return &actor->address;
}

caValue* actor_label(Actor* actor)
{
    return &actor->label;
}

void actors_dump(ActorSpace* space)
{
    std::cout << "[ActorSpace #" << space->id << "]" << std::endl;
    std::cout << space->currentInbox << "," << space->nextInbox << std::endl;
    std::cout << " Inboxes:" << std::endl;
    for (int i=0; i < list_length(&space->actorList); i++) {
        Actor* actor = as_actor(list_get(&space->actorList, i));
        std::cout << "  " << to_string(&actor->address) << ": current "
            << to_string(&actor->inbox[space->currentInbox])
            << ", next " << to_string(&actor->inbox[space->nextInbox])
            << std::endl;
        std::cout << "   nextIndex = " << actor->nextIndex << std::endl;
    }
}

CIRCA_EXPORT caActorSpace* circa_create_actor_space(caWorld* world)
{
    return create_actor_space(world);
}

CIRCA_EXPORT caActor* circa_create_actor(caActorSpace* space)
{
    return create_actor(space);
}
CIRCA_EXPORT void circa_actor_bind_name(caActorSpace* space, caActor* actor, const char* name)
{
    Value nameVal;
    set_string(&nameVal, name);
    return actor_bind_name(space, actor, &nameVal);
}

CIRCA_EXPORT void circa_actors_run_iteration(caActorSpace* space)
{
    return actors_run_iteration(space);
}

CIRCA_EXPORT bool circa_actor_consume_incoming(Actor* actor, caValue* messageOut)
{
    return actor_consume_incoming(actor, messageOut);
}
CIRCA_EXPORT caValue* circa_actor_post(caActor* actor)
{
    return actor_post(actor);
}
CIRCA_EXPORT caValue* circa_post(caActorSpace* space, caValue* address)
{
    return actor_post(space, address);
}
#endif

} // namespace circa

