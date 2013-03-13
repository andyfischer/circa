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

    // Runtime errors are sent to errorService.
    Value errorService;

    World* world;
};

ActorSpace* create_actor_space(World* world)
{
    ActorSpace* space = new ActorSpace();
    space->world = world;
    space->currentInbox = 0;
    space->nextInbox = 1;
    set_list(&space->actors, 0);
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
    actor->stack = alloc_stack(space->world);
    set_list(&actor->inbox[0], 0);
    set_list(&actor->inbox[1], 0);

    set_actor(list_append(&space->actors), actor);

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
    for (int i=0; i < list_length(&space->actors); i++) {
        Actor* actor = as_actor(list_get(&space->actors), i);
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
    return frame_by_id
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

} // namespace circa
