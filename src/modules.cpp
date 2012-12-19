// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "file.h"
#include "file_watch.h"
#include "function.h"
#include "inspection.h"
#include "importing.h"
#include "list.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "native_patch.h"
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
    set_string(world->moduleSearchPaths.append(), str);
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

Block* find_loaded_module(const char* name)
{
    for (BlockIteratorFlat it(global_root_block()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.module && term->name == name)
            return nested_contents(term);
    }
    return NULL;
}

Block* fetch_module(World* world, const char* name)
{
    Block* existing = find_module(world, name);
    if (existing != NULL)
        return existing;

    Term* term = apply(world->root, FUNCS.module, TermList(), name_from_string(name));
    return nested_contents(term);
}

static bool find_module_file(World* world, const char* module_name, caValue* filenameOut)
{
    Value module;
    set_string(&module, module_name);

    int count = list_length(&world->moduleSearchPaths);
    for (int i=0; i < count; i++) {

        caValue* searchPath = list_get(&world->moduleSearchPaths, i);

        // For each search path we'll check two places.

        // Look under searchPath/moduleName.ca
        Value computedPath;
        copy(searchPath, &computedPath);
        join_path(&computedPath, &module);
        string_append(&computedPath, ".ca");

        if (circa_file_exists(as_cstring(&computedPath))) {
            move(&computedPath, filenameOut);
            return true;
        }

        // Look under searchPath/moduleName/moduleName.ca
        copy(searchPath, &computedPath);

        join_path(&computedPath, &module);
        join_path(&computedPath, &module);
        string_append(&computedPath, ".ca");

        if (circa_file_exists(as_cstring(&computedPath))) {
            move(&computedPath, filenameOut);
            return true;
        }
    }
    return false;
}

Block* load_module_file(World* world, const char* moduleName, const char* filename)
{
    Block* existing = find_module(world, moduleName);

    if (existing == NULL) {
        Term* term = apply(world->root, FUNCS.module, TermList(),
                name_from_string(moduleName));
        existing = nested_contents(term);
    }

    Block* newBlock = alloc_block_gc();
    block_graft_replacement(existing, newBlock);
    load_script(newBlock, filename);

    update_static_error_list(newBlock);

    if (existing != NULL) {
        // New block starts off with the old block's version, plus 1.
        newBlock->version = existing->version + 1;

        update_world_after_module_reload(world, existing, newBlock);
    }

    return newBlock;
}

Block* load_module_file_watched(World* world, const char* moduleName, const char* filename)
{
    // Load and parse the script file.
    Block* block = load_module_file(world, moduleName, filename);

    // Create implicit file watch.
    FileWatch* watch = add_file_watch_module_load(world, filename, moduleName);

    // Since we just loaded the script, update the file watch.
    file_watch_ignore_latest_change(watch);

    return block;
}

Block* load_module_by_name(World* world, const char* moduleName)
{
    Block* existing = find_loaded_module(moduleName);
    if (existing != NULL)
        return existing;
    
    Value filename;
    bool found = find_module_file(world, moduleName, &filename);

    if (!found)
        return NULL;

    return load_module_file_watched(world, moduleName, as_cstring(&filename));
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

        if (string_eq(blockFilename, filename))
            return nested_contents(term);
    }

    return NULL;
}

// Returns the corresponding term inside newBlock, if found.
// Returns 'term' if the translation does not apply (term is not found inside
// oldBlock).
// Returns NULL if the translation does apply, but a corresponding term cannot be found.
Term* translate_term_across_blockes(Term* term, Block* oldBlock, Block* newBlock)
{
    if (!term_is_child_of_block(term, oldBlock))
        return term;

    Value relativeName;
    get_relative_name_as_list(term, oldBlock, &relativeName);
    return find_from_relative_name_list(&relativeName, newBlock);
}

void update_all_code_references(Block* target, Block* oldBlock, Block* newBlock)
{
    ca_assert(target != oldBlock);
    ca_assert(target != newBlock);

    // Store a cache of lookups that we've made in this call.
    TermMap cache;

    for (BlockIterator it(target); it.unfinished(); it.advance()) {

        Term* term = *it;

        // Iterate through each "dependency", which includes the function & inputs.
        for (int i=0; i < term->numDependencies(); i++) {
            Term* ref = term->dependency(i);
            Term* newRef = NULL;

            if (cache.contains(ref)) {
                newRef = cache[ref];
            } else {

                // Lookup and save result in cache
                newRef = translate_term_across_blockes(ref, oldBlock, newBlock);
                cache[ref] = newRef;
            }

            // Possibly rebind
            if (newRef != ref)
                term->setDependency(i, newRef);
        }
    }
}

void require_func_postCompile(Term* term)
{
    caValue* moduleName = term_value(term->input(0));
    Block* module = load_module_by_name(global_world(), as_cstring(moduleName));
    if (module != NULL)
        module_on_loaded_by_term(module, term);
}

void import_file_func_postCompile(Term* term)
{
    caValue* moduleName = term_value(term->input(0));
    caValue* filename = term_value(term->input(1));
    Block* module = load_module_file_watched(global_world(), as_cstring(moduleName),
            as_cstring(filename));
    module_on_loaded_by_term(module, term);
}

void native_patch_this_postCompile(Term* term)
{
    Block* block = term->owningBlock;
    Value blockName;
    get_global_name(block->owningTerm, &blockName);

    if (!is_string(&blockName)) {
        std::cout << "term doesn't have global name in native_patch_this_postCompile"
            << std::endl;
        return;
    }

    // Fetch the native module's filename, this might require parse-time eval.
    Term* filenameInput = term->input(0);
    Value filename;
    evaluate_minimum2(filenameInput, &filename);

    if (!is_string(&filename)) {
        std::cout << "input is not a string value in native_patch_this_postCompile"
            << std::endl;
        return;
    }

    string_remove_suffix(&filename, ".ca");
    native_patch_add_platform_specific_suffix(&filename);

    // Add a file watch that will update the NativePatch on file change.
    FileWatch* watch = add_file_watch_native_patch(global_world(),
            as_cstring(&filename), as_cstring(&blockName));

    NativePatch* module = add_native_patch(global_world(), as_cstring(&blockName));

    file_watch_check_now(global_world(), watch);
}

void modules_install_functions(Block* kernel)
{
    FUNCS.require = install_function(kernel, "require", NULL);
    as_function(FUNCS.require)->postCompile = require_func_postCompile;

    FUNCS.package = install_function(kernel, "package", NULL);

    Term* import_file = install_function(kernel, "import_file", NULL);
    as_function(import_file)->postCompile = import_file_func_postCompile;

    Term* native_patch_this = install_function(kernel, "native_patch_this", NULL);
    as_function(native_patch_this)->postCompile = native_patch_this_postCompile;

    FUNCS.module = import_function(kernel, NULL, "module() -> Block");
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

CIRCA_EXPORT caBlock* circa_load_module_from_file(caWorld* world, const char* module_name,
        const char* filename)
{
    return (caBlock*) load_module_file_watched(world, module_name, filename);
}

} // namespace circa
