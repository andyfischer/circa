// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "actors.h"
#include "interpreter.h"
#include "kernel.h"
#include "tagged_value.h"
#include "type.h"
#include "world.h"

namespace circa {

struct Actor {
    ActorSpace* space;
    Block* block;
    Stack* stack;
    Value inbox[2];
};

struct ActorSpace {
    int currentInbox;
    int nextInbox;

    Value actorList;

    Value moduleLoader;
    Value errorListener;

    World* world;
};

ActorSpace* create_actor_space(World* world)
{
    ActorSpace* space = new ActorSpace();
    space->world = world;
    space->currentInbox = 0;
    space->nextInbox = 1;
    set_list(&space->actorList, 0);
    return space;
}

void free_actor_space(ActorSpace* space)
{
    delete space;
}

void set_actor_for_role(ActorSpace* space, Actor* actor, const char* role)
{
    caValue* roleSlot = NULL;

    if (strcmp(role, "moduleLoader") == 0)
        roleSlot = &space->moduleLoader;
    else if (strcmp(role, "errorListener") == 0)
        roleSlot = &space->moduleLoader;
    else
        internal_error("didn't recognize role name");

    set_actor(roleSlot, actor);
}

Actor* create_actor(ActorSpace* space)
{
    Actor* actor = new Actor();
    actor->space = space;
    actor->block = NULL;
    actor->stack = alloc_stack(space->world);
    set_list(&actor->inbox[0], 0);
    set_list(&actor->inbox[1], 0);

    set_actor(list_append(&space->actorList), actor);

    return actor;
}

void free_actor(Actor* actor)
{
    delete actor;
}

void actors_start_iteration(ActorSpace* space)
{
    space->currentInbox = space->nextInbox;
    space->nextInbox = space->nextInbox == 0 ? 1 : 0;
}

void actors_run_iteration(ActorSpace* space)
{
    for (int i=0; i < list_length(&space->actorList); i++) {
        Actor* actor = as_actor(list_get(&space->actorList, i));
        if (actor_has_incoming(actor)) {
            Value messages;
            actor_consume_incoming(actor, &messages);
        }
    }
}

void actor_run(Actor* actor)
{
    // Fetch state output
    // Inject messages into primary input
    // Run interpreter
}

void actor_set_block(Actor* actor, Block* block)
{
    actor->block = block;

    stack_reset(actor->stack);

    push_frame(actor->stack, block);
}

bool actor_has_incoming(Actor* actor)
{
    return list_empty(&actor->inbox[actor->space->currentInbox]);
}

void actor_consume_incoming(Actor* actor, caValue* messagesOut)
{
    caValue* inbox = &actor->inbox[actor->space->currentInbox];
    move(inbox, messagesOut);
    set_list(inbox, 0);
}

caValue* actor_incoming_message_slot(Actor* actor)
{
    ca_assert(top_frame_parent(actor->stack) == NULL);
    return frame_register(top_frame(actor->stack), 0);
}

caValue* actor_post(Actor* actor)
{
    caValue* inbox = &actor->inbox[actor->space->nextInbox];
    return list_append(inbox);
}

void set_actor(caValue* value, Actor* actor)
{
    change_type(value, TYPES.actor);
    value->value_data.ptr = actor;
}

bool is_actor(caValue* value)
{
    return value->value_type == TYPES.actor;
}

Actor* as_actor(caValue* value)
{
    ca_assert(is_actor(value));
    return (Actor*) value->value_data.ptr;
}

void actor_setup_type(Type* type)
{
    set_string(&type->name, "Actor");
    type->storageType = sym_StorageTypeObject;
}

CIRCA_EXPORT caActorSpace* circa_create_actor_space(caWorld* world)
{
    return create_actor_space(world);
}

CIRCA_EXPORT caActor* circa_create_actor(caActorSpace* space)
{
    return create_actor(space);
}

CIRCA_EXPORT void circa_set_actor_for_role(caActorSpace* space, caActor* actor, const char* role)
{
    return set_actor_for_role(space, actor, role);
}

} // namespace circa
