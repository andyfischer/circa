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
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"
#include "vm.h"

namespace circa {

Value* module_search_paths(World* world)
{
    return &world->moduleSearchPaths;
}

void module_add_search_path(World* world, const char* str)
{
    set_string(list_append(&world->moduleSearchPaths), str);
}

Block* find_module_local(World* world, Value* relativeDir, Value* name)
{
    Value filename;
    resolve_possible_module_path(world, relativeDir, name, &filename);

    if (is_null(&filename))
        return NULL;

    return find_module_by_filename(world, &filename);
}

Block* find_module(World* world, Value* relativeDir, Value* name)
{
    stat_increment(FindModule);

    if (relativeDir != NULL && !is_null(relativeDir))
        return find_module_local(world, relativeDir, name);

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

CIRCA_EXPORT void circa_resolve_possible_module_path(World* world, Value* dir, Value* moduleName, Value* result)
{
    Value path;
    copy(dir, &path);

    if (moduleName != NULL)
        join_path(&path, moduleName);

    // try original path
    if (circa_file_exists(world, as_cstring(&path))) {
        copy(&path, result);
        return;
    }

    // try <path>.ca
    copy(&path, result);

    string_append(result, ".ca");
    if (circa_file_exists(world, as_cstring(result)))
        return;
        
    // try <path>/main.ca
    copy(&path, result);

    Value mainStr;
    set_string(&mainStr, "main.ca");
    join_path(result, &mainStr);

    if (circa_file_exists(world, as_cstring(result)))
        return;

    // not found
    set_null(result);
}

Value* find_enclosing_filename(Block* block)
{
    if (block == NULL)
        return NULL;

    Value* filename = block_get_property(block, s_filename);

    if (filename != NULL)
        return filename;

    return find_enclosing_filename(get_parent_block(block));
}

void find_enclosing_dirname(Block* block, Value* result)
{
    Value* filename = find_enclosing_filename(block);
    if (filename == NULL) {
        set_null(result);
        return;
    }

    get_directory_for_filename(filename, result);
}

void find_module_file_local(World* world, Block* loadedBy, Value* moduleName, Value* filenameOut)
{
    Value* loadedByFilename = find_enclosing_filename(loadedBy);

    if (loadedByFilename != NULL)
        resolve_possible_module_path(world, loadedByFilename, moduleName, filenameOut);
}

void find_module_file_global(World* world, Value* moduleName, Value* filenameOut)
{
    int count = list_length(&world->moduleSearchPaths);
    for (int i=0; i < count; i++) {

        Value* searchPath = list_get(&world->moduleSearchPaths, i);
        resolve_possible_module_path(world, searchPath, moduleName, filenameOut);
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

CIRCA_EXPORT Block* circa_load_module_by_filename(World* world, Value* filename)
{
    Value resolved;
    resolve_possible_module_path(world, filename, NULL, &resolved);
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

Block* load_module(World* world, Value* relativeDir, Value* moduleName)
{
    Value filename;

    if (relativeDir != NULL && !is_null(relativeDir)) {
        resolve_possible_module_path(world, relativeDir, moduleName, &filename);

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

Block* load_module_relative_to(World* world, Block* relativeTo, Value* moduleName)
{
    return load_module(world, find_enclosing_filename(relativeTo), moduleName);
}

Block* module_ref_resolve(World* world, Value* moduleRef)
{
    Value* name = moduleRef->index(0);
    Value* relativeDir = moduleRef->index(1);
    return find_module(world, relativeDir, name);
}

bool is_module_ref(Value* value)
{
    return value->value_type == TYPES.module_ref;
}

void set_module_ref(Value* value, Value* name, Value* relativeDir)
{
    make(TYPES.module_ref, value);
    set_value(value->index(0), name);
    if (relativeDir != NULL)
        set_value(value->index(1), relativeDir);
}

Term* module_lookup(Block* module, Term* caller)
{
    Value* elementName = caller->getProp(s_method_name);
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


CIRCA_EXPORT void circa_add_module_search_path(caWorld* world, const char* path)
{
    module_add_search_path(world, path);
}

CIRCA_EXPORT caBlock* circa_load_module_from_file(caWorld* world, const char* moduleName,
        const char* filename)
{
    return NULL;
}

CIRCA_EXPORT caBlock* circa_load_module(caWorld* world, const char* moduleName)
{
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module(world, NULL, &moduleNameVal);
}

} // namespace circa
