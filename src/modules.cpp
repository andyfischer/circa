// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "branch.h"
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
#include "native_modules.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "world.h"

namespace circa {

List g_moduleSearchPaths;

List* modules_get_search_paths()
{
    return &g_moduleSearchPaths;
}

void modules_add_search_path(const char* str)
{
    if (strcmp(str, "") == 0)
        internal_error("blank path in modules_add_search_path");

    set_string(g_moduleSearchPaths.append(), str);
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

Branch* find_loaded_module(const char* name)
{
    for (BranchIteratorFlat it(global_root_branch()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.imported_file && term->name == name)
            return nested_contents(term);
    }
    return NULL;
}

Branch* load_module_from_file(const char* module_name, const char* filename)
{
    Term* import = apply(global_root_branch(), FUNCS.imported_file, TermList(), name_from_string(module_name));
    load_script(nested_contents(import), filename);
    return nested_contents(import);
}

static bool find_module_file(const char* module_name, caValue* filenameOut)
{
    Value module;
    set_string(&module, module_name);

    int count = list_length(&g_moduleSearchPaths);
    for (int i=0; i < count; i++) {

        // For each search path we'll check two places.

        // Look under searchPath/moduleName.ca
        Value searchPath;
        copy(g_moduleSearchPaths[i], &searchPath);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }

        // Look under searchPath/moduleName/moduleName.ca
        copy(g_moduleSearchPaths[i], &searchPath);

        join_path(&searchPath, &module);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }
    }
    return false;
}

Branch* load_module_from_file2(World* world, const char* moduleName, const char* filename,
        Term* loadCall)
{
    // Load and parse the script file.
    Branch* branch = load_script_to_global_name(world, filename, moduleName);

    // If a loadCall is provided, possibly move the new import to be before the loadCall.
    if (loadCall != NULL) {
        Term* moduleTerm = branch->owningTerm;

        Term* callersModule = find_parent_term_in_branch(loadCall, moduleTerm->owningBranch);

        if (callersModule != NULL && (moduleTerm->index > callersModule->index))
            move_before(moduleTerm, callersModule);
    }

    // Create implicit file watch.
    FileWatch* watch = add_file_watch_module_load(world, filename, moduleName);

    // Since we just loaded the script, update the file watch.
    file_watch_ignore_latest_change(watch);

    return branch;
}

Branch* load_module(World* world, const char* moduleName, Term* loadCall)
{
    Branch* existing = find_loaded_module(moduleName);
    if (existing != NULL)
        return existing;
    
    Value filename;
    bool found = find_module_file(moduleName, &filename);

    if (!found)
        return NULL;

    return load_module_from_file2(world, moduleName, as_cstring(&filename), loadCall);
}

Branch* find_module_from_filename(const char* filename)
{
    // O(n) search for a module with this filename. Could stand to be more efficient.
    for (int i=0; i < global_root_branch()->length(); i++) {
        Term* term = global_root_branch()->get(i);
        if (term->nestedContents == NULL)
            continue;

        caValue* branchFilename = branch_get_source_filename(nested_contents(term));
        if (branchFilename == NULL)
            continue;

        if (string_eq(branchFilename, filename))
            return nested_contents(term);
    }

    return NULL;
}

// Returns the corresponding term inside newBranch, if found.
// Returns 'term' if the translation does not apply (term is not found inside
// oldBranch).
// Returns NULL if the translation does apply, but a corresponding term cannot be found.
Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch)
{
    if (!term_is_child_of_branch(term, oldBranch))
        return term;

    Value relativeName;
    get_relative_name_as_list(term, oldBranch, &relativeName);
    return find_from_relative_name_list(&relativeName, newBranch);
}

void update_all_code_references(Branch* target, Branch* oldBranch, Branch* newBranch)
{
    ca_assert(target != oldBranch);
    ca_assert(target != newBranch);

    // Store a cache of lookups that we've made in this call.
    TermMap cache;

    for (BranchIterator it(target); it.unfinished(); it.advance()) {

        Term* term = *it;

        // Iterate through each "dependency", which includes the function & inputs.
        for (int i=0; i < term->numDependencies(); i++) {
            Term* ref = term->dependency(i);
            Term* newRef = NULL;

            if (cache.contains(ref)) {
                newRef = cache[ref];
            } else {

                // Lookup and save result in cache
                newRef = translate_term_across_branches(ref, oldBranch, newBranch);
                cache[ref] = newRef;
            }

            // Possibly rebind
            if (newRef != ref)
                term->setDependency(i, newRef);
        }
    }
}

void import_func_postCompile(Term* term)
{
    caValue* moduleName = term_value(term->input(0));
    load_module(global_world(), as_cstring(moduleName), term);
}

void import_file_func_postCompile(Term* term)
{
    caValue* moduleName = term_value(term->input(0));
    caValue* filename = term_value(term->input(1));
    load_module_from_file2(global_world(), as_cstring(moduleName), as_cstring(filename), term);
}

void native_patch_this_postCompile(Term* term)
{
    Branch* branch = term->owningBranch;
    Value branchGlobalName;
    get_global_name(branch->owningTerm, &branchGlobalName);

    if (!is_string(&branchGlobalName)) {
        std::cout << "term doesn't have global name in native_patch_this_postCompile"
            << std::endl;
        return;
    }

    Value filename;
    evaluate_minimum2(term, &filename);

    if (!is_string(&filename)) {
        std::cout << "input is not a string value in native_patch_this_postCompile"
            << std::endl;
        return;
    }

    string_remove_suffix(&filename, ".ca");
    native_module_add_platform_specific_suffix(&filename);

    FileWatch* watch = add_file_watch_native_patch(global_world(),
            as_cstring(&filename), as_cstring(&branchGlobalName));
    file_watch_check_now(global_world(), watch);
}

void modules_install_functions(Branch* kernel)
{
    FUNCS.import = install_function(kernel, "import", NULL);
    as_function(FUNCS.import)->postCompile = import_func_postCompile;

    Term* import_file = install_function(kernel, "import_file", NULL);
    as_function(import_file)->postCompile = import_file_func_postCompile;

    Term* native_patch_this = install_function(kernel, "native_patch_this", NULL);
    as_function(native_patch_this)->postCompile = native_patch_this_postCompile;
}

CIRCA_EXPORT void circa_run_module(caStack* stack, const char* moduleName)
{
    circa::Branch* branch = nested_contents(get_global(moduleName));

    evaluate_branch((circa::Stack*) stack, branch);
}

CIRCA_EXPORT void circa_add_module_search_path(caWorld* world, const char* path)
{
    modules_add_search_path(path);
}

CIRCA_EXPORT caBranch* circa_load_module_from_file(caWorld*, const char* module_name, const char* filename)
{
    return (caBranch*) load_module_from_file(module_name, filename);
}

} // namespace circa
