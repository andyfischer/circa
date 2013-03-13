// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Actor;
struct ActorSpace;

ActorSpace* create_actor_space(World* world);
void free_actor_space(ActorSpace* space);
Actor* create_actor(ActorSpace* space);
void free_actor(Actor* actor);

void actors_start_iteration(ActorSpace* space);
void actors_run_iteration(ActorSpace* space);

void actor_set_block(Actor* actor, Block* block);
bool actor_has_incoming(Actor* actor);
void actor_consume_incoming(Actor* actor, caValue* messagesOut);
caValue* actor_incoming_message_slot(Actor* actor);

void set_actor(caValue* value, Actor* actor);
bool is_actor(caValue* value);
Actor* as_actor(caValue* value);
void actor_setup_type(Type* type);

} // namespace circa
