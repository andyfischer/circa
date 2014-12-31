// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "file_source.h"
#include "interpreter.h"
#include "file_watch.h"
#include "hashtable.h"
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

#if CIRCA_ENABLE_LIBUV
  #include "ext/libuv.h"
#endif

#include "ext/perlin.h"

namespace circa {

World* alloc_world()
{
    World* world = (World*) malloc(sizeof(World));
    memset(world, 0, sizeof(World));

    world->nextTermID = 1;
    world->nextBlockID = 1;
    world->nextStackID = 1;
    world->nextTypeID = 1;
    world->firstStack = NULL;
    world->lastStack = NULL;
    world->builtinPatch = NULL;
    world->globalScriptVersion = 1;

    world->nativePatch = NULL;
    world->nativePatchCount = 0;
    world->funcTable = NULL;
    world->funcTableCount = 0;

    return world;
}

void dealloc_world(World* world)
{
    // TODO: free NativePatch contents
    free(world->nativePatch);
    free(world->funcTable);
    free(world);
}

void world_initialize(World* world)
{
    initialize_null(&world->modulesByName);
    initialize_null(&world->modulesByFilename);
    initialize_null(&world->everyModule);
    initialize_null(&world->fileSources);
    initialize_null(&world->moduleSearchPaths);

    set_hashtable(&world->modulesByName);
    set_hashtable(&world->modulesByFilename);
    set_list(&world->everyModule);
    set_list(&world->moduleSearchPaths);
    set_list(&world->fileSources);

    world->moduleSearchPaths.append_str("$builtins");

    world->fileWatchWorld = alloc_file_watch_world();
    world->builtinPatch = circa_create_native_patch(world, "builtins");

    #if CIRCA_ENABLE_LIBUV
        world->libuvWorld = alloc_libuv_world();
    #endif

    perlin_init();
}

void world_uninitialize(World* world)
{
    set_null(&world->moduleSearchPaths);
    free_file_watch_world(world->fileWatchWorld);
}

World* create_world()
{
    World* world = alloc_world();
    world_initialize(world);
    return world;
}

World* get_world(Term* term)
{
    return term->owningBlock->world;
}

void world_clear_file_sources(World* world)
{
    set_list(&world->fileSources, 0);
}

Value* world_append_file_source(World* world)
{
    return list_append(&world->fileSources);
}

void world_tick(caWorld* world)
{
    file_watch_check_all(world);
    #if CIRCA_ENABLE_LIBUV
        libuv_process_events(world->libuvWorld);
    #endif
}

extern "C" {

void circa_use_local_filesystem(caWorld* world, const char* rootDir)
{
    world_clear_file_sources(world);
    file_source_create_using_filesystem(world_append_file_source(world), rootDir);
}

void circa_use_tarball_filesystem(caWorld* world, Value* tarball)
{
    world_clear_file_sources(world);
    file_source_create_from_tarball(world_append_file_source(world), tarball);
}

void circa_use_in_memory_filesystem(caWorld* world)
{
    world_clear_file_sources(world);
    set_hashtable(world_append_file_source(world));
}

void circa_load_file_in_memory(caWorld* world, Value* filename, Value* contents)
{
    touch(&world->fileSources);
    for (int i=0; i < world->fileSources.length(); i++) {
        Value* fileSource = world->fileSources.index(i);
        if (is_hashtable(fileSource)) {
            Value entry;
            set_list(&entry, 2);
            entry.set_element_int(0, 0);
            copy(contents, entry.index(1));
            move(&entry, hashtable_insert(fileSource, filename));
            break;
        }
    }
}

void circa_set_log_handler(caWorld* world, void* context, caLogFunc func)
{
    world->logContext = context;
    world->logFunc = func;
}

} // extern "C"

} // namespace circa
