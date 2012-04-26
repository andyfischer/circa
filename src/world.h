// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {
    circa::Value actorList;
    caStack* mainStack;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
caValue* find_actor(World* world, const char* name);
void actor_post_message(caValue* actor, caValue* message);

} // namespace circa
