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

caValue* module_search_paths(World* world)
{
    return &world->moduleSearchPaths;
}

void module_add_search_path(World* world, const char* str)
{
    set_string(list_append(&world->moduleSearchPaths), str);
}

void module_get_default_name_from_filename(caValue* filename, caValue* moduleNameOut)
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

Block* find_loaded_module(caValue* name)
{
    for (BlockIteratorFlat it(global_root_block()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module && equals(term_name(term), name))
            return nested_contents(term);
    }
    return NULL;
}

Block* fetch_module(World* world, const char* name)
{
    Value nameStr;
    set_string(&nameStr, name);
    Block* existing = find_module(world->root, &nameStr);
    if (existing != NULL)
        return existing;

    return create_module(world, name);
}

Block* create_module(World* world, const char* name)
{
    Term* term = apply(world->root, FUNCS.module, TermList(), name);
    return nested_contents(term);
}

static bool check_module_search_path(World* world, caValue* moduleName,
    caValue* searchPath, caValue* filenameOut)
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

static bool find_module_file(World* world, Block* loadedBy,
    caValue* moduleName, caValue* filenameOut)
{
    // Check directory local to load location.
    if (loadedBy != NULL) {
        caValue* loadedByFilename = block_get_property(loadedBy, sym_Filename);
        if (loadedByFilename != NULL) {
            Value directory;
            get_directory_for_filename(loadedByFilename, &directory);
            if (check_module_search_path(world, moduleName, &directory, filenameOut))
                return true;
        }
    }

    int count = list_length(&world->moduleSearchPaths);
    for (int i=0; i < count; i++) {

        caValue* searchPath = list_get(&world->moduleSearchPaths, i);

        if (check_module_search_path(world, moduleName, searchPath, filenameOut))
            return true;
    }
    return false;
}

Block* load_module_file(World* world, caValue* moduleName, const char* filename)
{
    Block* existing = find_module(world->root, moduleName);
    Block* newBlock;

    if (existing == NULL) {
        newBlock = create_module(world, as_cstring(moduleName));
    } else {
        newBlock = alloc_block();
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

void install_block_as_module(World* world, caValue* moduleName, Block* block)
{
    Block* existing = find_module(world->root, moduleName);

    if (existing == NULL) {
        existing = create_module(world, as_cstring(moduleName));
        block_graft_replacement(existing, block);

    } else {
        block_graft_replacement(existing, block);

        Migration migration;
        migration.oldBlock = existing;
        migration.newBlock = block;
        migrate_world(world, &migration);
    }
}

Block* load_module_file_watched(World* world, caValue* moduleName, const char* filename)
{
    // Load and parse the script file.
    Block* block = load_module_file(world, moduleName, filename);

    // Create implicit file watch.
    FileWatch* watch = add_file_watch_module_load(world, filename, moduleName);

    // Since we just loaded the script, update the file watch.
    file_watch_ignore_latest_change(watch);

    return block;
}

Block* load_module_by_name(World* world, Block* loadedBy, caValue* moduleName)
{
    Block* existing = find_loaded_module(moduleName);
    if (existing != NULL)
        return existing;
    
    Value filename;
    bool found = find_module_file(world, loadedBy, moduleName, &filename);

    if (found)
        return load_module_file_watched(world, moduleName, as_cstring(&filename));

    const char* builtinText = find_builtin_module(as_cstring(moduleName));

    if (builtinText != NULL) {
        Block* newBlock = create_module(world, as_cstring(moduleName));
        load_script_from_text(newBlock, builtinText);
        return newBlock;
    }

    return NULL;
}

void module_on_loaded_by_term(Block* module, Term* loadCall)
{
    Term* moduleTerm = module->owningTerm;

    Term* callersModule = find_parent_term_in_block(loadCall, moduleTerm->owningBlock);

    if (callersModule != NULL && (moduleTerm->index > callersModule->index))
        move_before(moduleTerm, callersModule);
}

Block* find_module_from_filename(const char* filename)
{
    // O(n) search for a module with this filename. Could stand to be more efficient.
    for (int i=0; i < global_root_block()->length(); i++) {
        Term* term = global_root_block()->get(i);
        if (term->nestedContents == NULL)
            continue;

        caValue* blockFilename = block_get_source_filename(nested_contents(term));
        if (blockFilename == NULL)
            continue;

        if (string_equals(blockFilename, filename))
            return nested_contents(term);
    }

    return NULL;
}


void load_module_eval(caStack* stack)
{
    Term* caller = circa_caller_term(stack);
    caValue* moduleName = circa_input(stack, 0);
    Block* module = load_module_by_name(stack->world, caller->owningBlock, moduleName);
    set_block(circa_output(stack, 0), module);
}

void load_script_eval(caStack* stack)
{
    caValue* filename = circa_input(stack, 0);
    Value moduleName;
    set_string(&moduleName, "file:");
    string_append(&moduleName, filename);
    Block* module = load_module_file_watched(stack->world, &moduleName,
        as_cstring(filename));
    set_block(circa_output(stack, 0), module);
}

Block* find_module(Block* root, caValue* name)
{
    Block* moduleLevel = global_world()->root;

    for (int i=0; i < moduleLevel->length(); i++) {
        Term* term = moduleLevel->get(i);
        if (term->function != FUNCS.module)
            continue;

        if (equals(term_name(term), name))
            return nested_contents(term);
    }

    return NULL;
}

Block* module_ref_get_block(caValue* moduleRef)
{
    return as_block(list_get(moduleRef, 0));
}

bool is_module_ref(caValue* value)
{
    return value->value_type == TYPES.module_ref;
}

void require_eval(caStack* stack)
{
    caValue* moduleName = circa_input(stack, 0);
    Block* module = load_module_by_name(stack->world, circa_caller_block(stack), moduleName);

    if (module == NULL) {
        Value msg;
        set_string(&msg, "Couldn't find module named: ");
        string_append_quoted(&msg, moduleName);
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    if (has_static_errors_cached(module)) {
        Value msg;
        set_string(&msg, "Module '");
        string_append(&msg, moduleName);
        string_append(&msg, "' has static errors: \n");

        Value errors;
        format_static_errors(&module->staticErrors, &errors);
        string_append(&msg, &errors);
        circa_output_error(stack, as_cstring(&msg));
        return;
    }

    // Save a ModuleRef value.
    caValue* moduleRef = circa_output(stack, 0);
    make(TYPES.module_ref, moduleRef);
    set_block(list_get(moduleRef, 0), module);
}

void modules_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "require", require_eval);
    circa_patch_function(patch, "load_module", load_module_eval);
    circa_patch_function(patch, "load_script", load_script_eval);
}

CIRCA_EXPORT void circa_run_module(caStack* stack, const char* moduleName)
{
    circa::Block* block = nested_contents(find_global(moduleName));
    evaluate_block((circa::Stack*) stack, block);
}

CIRCA_EXPORT void circa_add_module_search_path(caWorld* world, const char* path)
{
    module_add_search_path(world, path);
}

CIRCA_EXPORT caBlock* circa_load_module_from_file(caWorld* world, const char* moduleName,
        const char* filename)
{
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module_file_watched(world, &moduleNameVal, filename);
}

CIRCA_EXPORT caBlock* circa_load_module(caWorld* world, Block* loadedBy, const char* moduleName)
{
    Value moduleNameVal;
    set_string(&moduleNameVal, moduleName);
    return load_module_by_name(world, loadedBy, &moduleNameVal);
}

} // namespace circa
