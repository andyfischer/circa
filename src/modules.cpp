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
#include "importing.h"
#include "interpreter.h"
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

void module_get_default_name_from_filename(Value* filename, Value* moduleNameOut)
{
    get_just_filename_for_path(filename, moduleNameOut);

    // Chop off everything after the last dot (if there is one)
    int dotPos = string_find_char_from_end(moduleNameOut, '.');
    if (dotPos != -1) {
        Value temp;
        copy(moduleNameOut, &temp);
        string_slice(&temp, 0, dotPos, moduleNameOut);
    }
}

Block* find_module(World* world, Value* name)
{
    stat_increment(FindModule);

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

#if 0
Block* fetch_module(World* world, const char* name)
{
    Value nameStr;
    set_string(&nameStr, name);
    Block* existing = find_module(world, &nameStr);
    if (existing != NULL)
        return existing;

    return create_module(world, name);
}
#endif

Block* create_module(World* world)
{
    Block* block = alloc_block(world);
    block_set_bool_prop(block, sym_IsModule, true);
    set_block(list_append(&world->everyModule), block);
    return block;
}

void module_set_name(World* world, Block* block, Value* name)
{
    copy(name, block_insert_property(block, sym_Name));
    set_block(hashtable_insert(&world->modulesByName, name), block);
}

void module_set_filename(World* world, Block* block, Value* filename)
{
    copy(filename, block_insert_property(block, sym_Filename));
    set_block(hashtable_insert(&world->modulesByFilename, filename), block);
}

void resolve_possible_module_path(World* world, Value* path, Value* result)
{
    if (circa_file_exists(world, as_cstring(path))) {
        copy(path, result);
        return;
    }

    copy(path, result);

    string_append(result, ".ca");
    if (circa_file_exists(world, as_cstring(result)))
        return;
        
    copy(path, result);

    Value packageStr;
    set_string(&packageStr, "package.ca");

    join_path(result, &packageStr);

    if (circa_file_exists(world, as_cstring(result)))
        return;

    set_null(result);
}

#if 0
static bool check_module_search_path(World* world, Value* moduleName,
    Value* searchPath, Value* filenameOut)
{
    // For each search path we'll check two places.

    // Look under searchPath/moduleName.ca
    Value computedPath;
    copy(searchPath, &computedPath);
    join_path(&computedPath, moduleName);
    string_append(&computedPath, ".ca");

    if (circa_file_exists(world, as_cstring(&computedPath))) {
        move(&computedPath, filenameOut);
        return true;
    }

    // Look under searchPath/moduleName/moduleName.ca
    copy(searchPath, &computedPath);

    join_path(&computedPath, moduleName);
    join_path(&computedPath, moduleName);
    string_append(&computedPath, ".ca");

    if (circa_file_exists(world, as_cstring(&computedPath))) {
        move(&computedPath, filenameOut);
        return true;
    }
    
    return false;
}
#endif

void find_module_file_local(World* world, Block* loadedBy, Value* moduleName, Value* filenameOut)
{
    Value* loadedByFilename = block_get_property(loadedBy, sym_Filename);
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

#if 0
Block* load_module_file(World* world, Value* moduleName, const char* filename)
{
    Block* existing = find_module(world, moduleName);
    Block* newBlock;

    if (existing == NULL) {
        newBlock = create_module(world, moduleName, NULL);
    } else {
        newBlock = alloc_block(world);
        block_graft_replacement(existing, newBlock);
    }

    load_script(newBlock, filename);

    if (existing != NULL) {
        Migration migration;
        migration.oldBlock = existing;
        migration.newBlock = newBlock;
        migrate_world(world, &migration);
    }

    return newBlock;
}
#endif

void module_install_replacement(World* world, Value* filename, Block* replacement)
{
    Block* existing = find_module_by_filename(world, filename);
    ca_assert(existing != NULL);
    Value* existingName = block_get_property(existing, sym_Name);

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

#if 0
Block* load_module_file_watched(World* world, Value* moduleName, const char* filename)
{
    // Load and parse the script file.
    Block* block = load_module_file(world, moduleName, filename);

    // Create implicit file watch.
    FileWatch* watch = add_file_watch_module_load(world, filename, moduleName);

    // Since we just loaded the script, update the file watch.
    file_watch_ignore_latest_change(watch);

    return block;
}
#endif

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

    Block* existing = find_module(world, moduleName);
    if (existing != NULL)
        return existing;

    find_module_file_global(world, moduleName, &filename);

    if (!is_null(&filename)) {
        Block* block = load_module_by_filename(world, &filename);
        module_set_name(world, block, moduleName);
        return block;
    }

    // Try as a builtin module
    const char* builtinText = find_builtin_module(as_cstring(moduleName));

    if (builtinText != NULL) {
        Block* newBlock = create_module(world);
        module_set_name(world, newBlock, moduleName);
        load_script_from_text(newBlock, builtinText);
        return newBlock;
    }

    return NULL;
}

Block* module_ref_get_block(Value* moduleRef)
{
    return as_block(list_get(moduleRef, 0));
}

bool is_module_ref(Value* value)
{
    return value->value_type == TYPES.module_ref;
}

void set_module_ref(Value* value, Block* block)
{
    make(TYPES.module_ref, value);
    set_block(list_get(value, 0), block);
}

Term* module_lookup(Value* module, Term* caller)
{
    Block* block = module_ref_get_block(module);
    Value* elementName = caller->getProp(sym_MethodName);
    Term* term = find_local_name(block, elementName);
    return term;
}
 
void load_script_eval(Stack* stack)
{
    Value* filename = circa_input(stack, 0);
    Block* block = load_module_by_filename(stack->world, filename);
    set_module_ref(circa_output(stack, 0), block);
}

void modules_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "load_script", load_script_eval);
}

CIRCA_EXPORT void circa_add_module_search_path(caWorld* world, const char* path)
{
    module_add_search_path(world, path);
}

CIRCA_EXPORT caBlock* circa_load_module_from_file(caWorld* world, const char* moduleName,
        const char* filename)
{
#if 0
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module_file_watched(world, &moduleNameVal, filename);
#endif
    return NULL;
}

CIRCA_EXPORT caBlock* circa_load_module(caWorld* world, Block* relativeTo, const char* moduleName)
{
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module(world, relativeTo, &moduleNameVal);
}

} // namespace circa
