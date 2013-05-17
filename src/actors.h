// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Actors v3

struct Actor {
    int id;
    Stack* stack;
};

Stack* create_actor(World* world, Block* block);
bool state_inject(Stack* stack, caValue* name, caValue* value);
caValue* context_inject(Stack* stack, caValue* name);

} // namespace circa
