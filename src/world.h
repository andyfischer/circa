// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {

    Block* builtins;
    Value modulesByName;
    Value modulesByFilename;
    Value everyModule;

    // Private data.
    FileWatchWorld* fileWatchWorld;
    LibuvWorld* libuvWorld;

    // Global IDs.
    int nextTermID;
    int nextBlockID;
    int nextStackID;
    int nextTypeID;

    // Active file sources.
    Value fileSources;

    // Module information.
    Value moduleSearchPaths;

    Stack* firstStack;
    Stack* lastStack;

    // Whether the world is currently bootstrapping. Either :Bootstrapping or :Done.
    Symbol bootstrapStatus;

    // NativePatch data.
    NativePatch** nativePatch;
    int nativePatchCount;

    NativeFunc* funcTable;
    int funcTableCount;

    // Special NativePatch for builtin funcs.
    NativePatch* builtinPatch;

    int globalScriptVersion;

    caLogFunc logFunc;
    void* logContext;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
void dealloc_world(World* world);
void world_initialize(World* world);
void world_uninitialize(World* world);
World* create_world();

World* get_world(Term* term);

void world_clear_file_sources(World* world);
Value* world_append_file_source(World* world);
void world_use_local_filesystem(World* world, const char* rootDir);

void world_tick(caWorld* world);

} // namespace circa
