// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {

    Block* root;

    // Private data.
    NativePatchWorld* nativePatchWorld;
    FileWatchWorld* fileWatchWorld;

    // Global IDs.
    int nextActorID;
    int nextTermID;
    int nextBlockID;
    int nextStackID;

    // Module information.
    List moduleSearchPaths;

    Stack* firstRootStack;
    Stack* lastRootStack;

    // Whether the world is currently bootstrapping. Either :Bootstrapping or :Done.
    Symbol bootstrapStatus;

    caLogFunc logFunc;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
void world_initialize(World* world);
World* create_world();

void refresh_all_modules(caWorld* world);

} // namespace circa
