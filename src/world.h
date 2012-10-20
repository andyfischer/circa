// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {

    Branch* root;

    circa::Value actorList;
    caStack* actorStack;

    NativeModuleWorld* nativeModuleWorld;
    FileWatchWorld* fileWatchWorld;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* create_world();
void world_initialize(World* world);

void actor_send_message(ListData* actor, caValue* message);
void actor_run_message(caStack* stack, ListData* actor, caValue* message);
ListData* find_actor(World* world, const char* name);

void refresh_all_modules(caWorld* world);
void load_branch(World* world, const char* moduleName, const char* filename);

} // namespace circa
