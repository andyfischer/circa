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

void world_initialize(World* world)
{
    set_list(&world->moduleSearchPaths);

    world->nativePatchWorld = create_native_patch_world();
    world->fileWatchWorld = create_file_watch_world();
}

World* create_world()
{
    World* world = alloc_world();
    world_initialize(world);
    return world;
}

} // namespace circa
