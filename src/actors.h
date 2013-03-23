// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Actors v3 (current)

struct Actor;
Actor* create_actor(World* world, Block* block);
void set_actor(caValue* value, Actor* actor);
bool is_actor(caValue* value);
Actor* as_actor(caValue* value);

bool actor_call_in_progress(Actor* actor);
caValue* actor_input_slot(Actor* actor);
caValue* actor_output_slot(Actor* actor);
bool actor_inject(Actor* actor, caValue* name, caValue* value);
void actor_run(Actor* actor);

void actor_setup_type(Type* type);

#if 0

// Actors v2:

struct Actor;
struct ActorSpace;

ActorSpace* create_actor_space(World* world);
void free_actor_space(ActorSpace* space);

Actor* create_actor(ActorSpace* space);
void free_actor(Actor* actor);

void actor_bind_name(ActorSpace* space, Actor* actor, caValue* name);

void actors_run_iteration(ActorSpace* space);
void actor_set_block(Actor* actor, Block* block);
bool actor_has_incoming(Actor* actor);
bool actor_consume_incoming(Actor* actor, caValue* message);
caValue* actor_incoming_message_slot(Actor* actor);
caValue* actor_post(Actor* actor);
caValue* actor_post(ActorSpace* space, caValue* address);

// Returns an actor's address (suitable for passing to actor_post). May not be modified.
caValue* actor_address(Actor* actor);

// Returns a user-defined label value. May be modified.
caValue* actor_label(Actor* actor);

void set_actor(caValue* value, Actor* actor);
bool is_actor(caValue* value);
Actor* as_actor(caValue* value);
void actor_setup_type(Type* type);

void actors_dump(ActorSpace* space);

#endif

} // namespace circa
