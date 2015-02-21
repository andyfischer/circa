// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "building.h"
#include "closures.h"
#include "code_iterators.h"
#include "file.h"
#include "file_watch.h"
#include "function.h"
#include "hashtable.h"
#include "inspection.h"
#include "list.h"
#include "kernel.h"
#include "migration.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
#include "reflection.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"

namespace circa {

Value* module_search_paths(World* world)
{
    return &world->moduleSearchPaths;
}

void module_add_search_path(World* world, const char* str)
{
    set_string(list_append(&world->moduleSearchPaths), str);
}

Block* find_module_local(World* world, Block* relativeTo, Value* name)
{
    Value filename;
    find_module_file_local(world, relativeTo, name, &filename);

    if (is_null(&filename))
        return NULL;

    return find_module_by_filename(world, &filename);
}

Block* find_module(World* world, Block* relativeTo, Value* name)
{
    stat_increment(FindModule);

    if (relativeTo != NULL)
        return find_module_local(world, relativeTo, name);

    Value* block = hashtable_get(&world->modulesByName, name);
    if (block == NULL)
        return NULL;
    return as_block(block);
}

Block* find_module_by_filename(World* world, Value* filename)
{
    Value* block = hashtable_get(&world->modulesByFilename, filename);
    if (block == NULL)
        return NULL;
    return as_block(block);
}

Block* create_module(World* world)
{
    Block* block = alloc_block(world);
    block_set_bool_prop(block, s_IsModule, true);
    set_block(list_append(&world->everyModule), block);
    return block;
}

void module_set_name(World* world, Block* block, Value* name)
{
    copy(name, block_insert_property(block, s_Name));
    set_block(hashtable_insert(&world->modulesByName, name), block);
}

void module_set_filename(World* world, Block* block, Value* filename)
{
    copy(filename, block_insert_property(block, s_filename));
    set_block(hashtable_insert(&world->modulesByFilename, filename), block);
}

void resolve_possible_module_path(World* world, Value* path, Value* result)
{
    // try original path
    if (circa_file_exists(world, as_cstring(path))) {
        copy(path, result);
        return;
    }

    // try <path>.ca
    copy(path, result);

    string_append(result, ".ca");
    if (circa_file_exists(world, as_cstring(result)))
        return;
        
    // try <path>/main.ca
    copy(path, result);

    Value mainStr;
    set_string(&mainStr, "main.ca");
    join_path(result, &mainStr);

    if (circa_file_exists(world, as_cstring(result)))
        return;

    // not found
    set_null(result);
}

void find_module_file_local(World* world, Block* loadedBy, Value* moduleName, Value* filenameOut)
{
    Value* loadedByFilename = block_get_property(loadedBy, s_filename);
    if (loadedByFilename != NULL) {
        Value path;
        get_directory_for_filename(loadedByFilename, &path);
        join_path(&path, moduleName);

        resolve_possible_module_path(world, &path, filenameOut);
        if (!is_null(filenameOut))
            return;
    }
}

void find_module_file_global(World* world, Value* moduleName, Value* filenameOut)
{
    int count = list_length(&world->moduleSearchPaths);
    for (int i=0; i < count; i++) {

        Value* searchPath = list_get(&world->moduleSearchPaths, i);

        Value path;
        copy(searchPath, &path);
        join_path(&path, moduleName);

        resolve_possible_module_path(world, &path, filenameOut);
        if (!is_null(filenameOut))
            return;
    }
}

void module_install_replacement(World* world, Value* filename, Block* replacement)
{
    Block* existing = find_module_by_filename(world, filename);
    ca_assert(existing != NULL);
    Value* existingName = block_get_property(existing, s_Name);

    for (int i=0; i < world->everyModule.length(); i++) {
        if (as_block(world->everyModule.index(i)) == existing)
            set_block(world->everyModule.index(i), replacement);
    }

    module_set_filename(world, replacement, filename);

    if (existingName != NULL)
        module_set_name(world, replacement, existingName);

    Migration migration;
    migration.oldBlock = existing;
    migration.newBlock = replacement;
    migrate_world(world, &migration);
}

Block* load_module_by_filename(World* world, Value* filename)
{
    Value resolved;
    resolve_possible_module_path(world, filename, &resolved);
    if (is_null(&resolved))
        return NULL;

    Block* block = find_module_by_filename(world, &resolved);
    if (block != NULL)
        return block;

    block = create_module(world);
    module_set_filename(world, block, &resolved);

    load_script(block, as_cstring(&resolved));

    FileWatch* watch = add_file_watch_recompile_module(world, as_cstring(&resolved));
    file_watch_ignore_latest_change(watch);

    return block;
}

Block* load_module(World* world, Block* relativeTo, Value* moduleName)
{
    Value filename;

    if (relativeTo != NULL) {
        Value filename;
        find_module_file_local(world, relativeTo, moduleName, &filename);

        if (!is_null(&filename))
            return load_module_by_filename(world, &filename);
    }

    Block* existing = find_module(world, NULL, moduleName);
    if (existing != NULL)
        return existing;

    find_module_file_global(world, moduleName, &filename);

    if (!is_null(&filename)) {
        Block* block = load_module_by_filename(world, &filename);
        module_set_name(world, block, moduleName);
        return block;
    }

    return NULL;
}

Block* module_ref_resolve(World* world, Value* moduleRef)
{
    Value* name = moduleRef->index(0);
    Block* relativeTo = as_block(moduleRef->index(1));
    return find_module(world, relativeTo, name);
}

bool is_module_ref(Value* value)
{
    return value->value_type == TYPES.module_ref;
}

void set_module_ref(Value* value, Value* path, Block* relativeTo)
{
    make(TYPES.module_ref, value);
    set_value(value->index(0), path);
    set_block(value->index(1), relativeTo);
}

Term* module_lookup(Block* module, Term* caller)
{
    Value* elementName = caller->getProp(s_MethodName);
    Term* term = find_local_name(module, elementName);
    return term;
}

Term* module_ref_lookup(Value* moduleRef, Term* caller)
{
    return module_lookup(module_ref_resolve(get_world(caller), moduleRef), caller);
}

Term* module_lookup(World* world, Value* moduleRef, Value* name)
{
    Block* module = module_ref_resolve(world, moduleRef);
    Term* term = find_local_name(module, name);
    return term;
}
 
#if 0
void load_script_eval(Stack* stack)
{
    Value* filename = circa_input(stack, 0);
    Block* block = load_module_by_filename(stack->world, filename);
    set_block(circa_output(stack, 0), block);
}
#endif

void modules_install_functions(NativePatch* patch)
{
#if 0
    circa_patch_function(patch, "load_script", load_script_eval);
#endif
}

CIRCA_EXPORT void circa_add_module_search_path(caWorld* world, const char* path)
{
    module_add_search_path(world, path);
}

CIRCA_EXPORT caBlock* circa_load_module_from_file(caWorld* world, const char* moduleName,
        const char* filename)
{
    return NULL;
}

CIRCA_EXPORT caBlock* circa_load_module(caWorld* world, Block* relativeTo, const char* moduleName)
{
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module(world, relativeTo, &moduleNameVal);
}

} // namespace circa
