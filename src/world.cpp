// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "interpreter.h"
#include "file_watch.h"
#include "kernel.h"
#include "inspection.h"
#include "list.h"
#include "modules.h"
#include "native_patch.h"
#include "reflection.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"

namespace circa {

World* alloc_world()
{
    World* world = (World*) malloc(sizeof(World));
    memset(world, 0, sizeof(World));

    world->nextActorID = 1;
    world->nextTermID = 1;
    world->nextBlockID = 1;
    world->nextStackID = 1;
    world->firstRootStack = NULL;
    world->lastRootStack = NULL;

    return world;
}

void dealloc_world(World* world)
{
    free(world);
}

void world_initialize(World* world)
{
    initialize_null(&world->fileSources);
    initialize_null(&world->moduleSearchPaths);

    set_list(&world->moduleSearchPaths);
    set_list(&world->fileSources);

    world->nativePatchWorld = create_native_patch_world();
    world->fileWatchWorld = create_file_watch_world();
}

void world_uninitialize(World* world)
{
    set_null(&world->moduleSearchPaths);
    dealloc_native_patch_world(world->nativePatchWorld);
    dealloc_file_watch_world(world->fileWatchWorld);
}

World* create_world()
{
    World* world = alloc_world();
    world_initialize(world);
    return world;
}

void world_clear_file_sources(World* world)
{
    set_list(&world->fileSources, 0);
}

void world_append_file_source(World* world, caValue* fileSource)
{
    copy(fileSource, list_append(&world->fileSources));
}

CIRCA_EXPORT void circa_use_local_filesystem(caWorld* world, const char* rootDir)
{
    Value fileSource;
    set_list(&fileSource, 2);
    set_symbol(list_get(&fileSource, 0), sym_Filesystem);
    set_string(list_get(&fileSource, 1), rootDir);
    world_append_file_source(world, &fileSource);
}

CIRCA_EXPORT void circa_set_log_handler(caWorld* world, void* context, caLogFunc func)
{
    world->logContext = context;
    world->logFunc = func;
}

} // namespace circa
