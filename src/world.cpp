// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "filepack.h"
#include "interpreter.h"
#include "file_watch.h"
#include "kernel.h"
#include "inspection.h"
#include "ext/libuv.h"
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
    world->nextTypeID = 1;
    world->firstStack = NULL;
    world->lastStack = NULL;
    world->builtinPatch = NULL;
    world->globalScriptVersion = 1;

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

    world->nativePatchWorld = alloc_native_patch_world();
    world->fileWatchWorld = alloc_file_watch_world();
    world->libuvWorld = alloc_libuv_world();

    world->builtinPatch = alloc_native_patch(world);
}

void world_uninitialize(World* world)
{
    set_null(&world->moduleSearchPaths);
    free_native_patch(world->builtinPatch);
    free_native_patch_world(world->nativePatchWorld);
    free_file_watch_world(world->fileWatchWorld);
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

caValue* world_append_file_source(World* world)
{
    return list_append(&world->fileSources);
}

void world_tick(caWorld* world)
{
    file_watch_check_all(world);
    libuv_process_events(world->libuvWorld);
}

extern "C" {

void circa_use_local_filesystem(caWorld* world, const char* rootDir)
{
    world_clear_file_sources(world);
    filepack_create_using_filesystem(world_append_file_source(world), rootDir);
}

void circa_use_tarball_filesystem(caWorld* world, caValue* tarball)
{
    world_clear_file_sources(world);
    filepack_create_from_tarball(world_append_file_source(world), tarball);
}

void circa_set_log_handler(caWorld* world, void* context, caLogFunc func)
{
    world->logContext = context;
    world->logFunc = func;
}

} // extern "C"

} // namespace circa
