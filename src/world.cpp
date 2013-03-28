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

    return world;
}

void world_initialize(World* world)
{
    set_list(&world->moduleSearchPaths);

    world->nativePatchWorld = create_native_patch_world();
    world->fileWatchWorld = create_file_watch_world();
}

void update_block_after_module_reload(Block* target, Block* oldBlock, Block* newBlock)
{
    // Noop if the target is our new block
    if (target == newBlock)
        return;

    ca_assert(target != oldBlock);

    update_all_code_references(target, oldBlock, newBlock);
}

void update_world_after_module_reload(World* world, Block* oldBlock, Block* newBlock)
{
    // Update references in every module
    for (BlockIteratorFlat it(world->root); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module) {
            update_block_after_module_reload(term->nestedContents, oldBlock, newBlock);
        }
    }
}

// TODO: Delete this, it was replaced by the file watch system.
void refresh_all_modules(caWorld* world)
{
    // Iterate over top-level modules
    for (BlockIteratorFlat it(world->root); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module) {
            
            Block* existing = term->nestedContents;
            Block* latest = load_latest_block(existing);

            if (existing != latest) {
                term->nestedContents = latest;

                update_world_after_module_reload(world, existing, latest);
            }
        }
    }
}

// TODO: delete this
CIRCA_EXPORT void circa_refresh_all_modules(caWorld* world)
{
    refresh_all_modules(world);
}

} // namespace circa
