// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {

    Block* root;

    // Actors.
    circa::Value actorList;
    caStack* actorStack;

    // Private data.
    NativeModuleWorld* nativeModuleWorld;
    FileWatchWorld* fileWatchWorld;

    // Next unused global IDs.
    int nextTermID;
    int nextBlockID;
    int nextStackID;

    // Module information.
    List moduleSearchPaths;

    // Whether the world is currently bootstrapping. Either Bootstrapping or Done.
    Name bootstrapStatus;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
void world_initialize(World* world);

void actor_send_message(ListData* actor, caValue* message);
void actor_run_message(caStack* stack, ListData* actor, caValue* message);
ListData* find_actor(World* world, const char* name);

void refresh_all_modules(caWorld* world);

void update_world_after_module_reload(World* world, Block* oldBlock, Block* newBlock);

} // namespace circa
