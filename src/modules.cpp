// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/file.h"

#include "building.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "filesystem.h"
#include "file_utils.h"
#include "list.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"

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

Term* find_loaded_module(const char* name)
{
    for (BranchIteratorFlat it(kernel()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.imported_file && term->name == name)
            return term;
    }
    return NULL;
}

Branch* load_module_from_file(const char* module_name, const char* filename)
{
    Term* import = apply(kernel(), FUNCS.imported_file, TermList(), module_name);
    load_script(nested_contents(import), filename);
    return nested_contents(import);
}

static bool find_module_file(const char* module_name, String* filenameOut)
{
    String module;
    set_string(&module, module_name);

    int count = list_length(&g_moduleSearchPaths);
    for (int i=0; i < count; i++) {

        // For each search path we'll check two places.

        // Look under searchPath/moduleName.ca
        String searchPath;
        copy(g_moduleSearchPaths[i], &searchPath);
        circa_join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }

        // Look under searchPath/moduleName/moduleName.ca
        copy(g_moduleSearchPaths[i], &searchPath);

        circa_join_path(&searchPath, &module);
        circa_join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (circa_file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }
    }
    return false;
}

caName load_module(const char* module_name, Term* loadCall)
{
    Term* existing = find_loaded_module(module_name);
    if (existing != NULL)
        return name_Success;
    
    String filename;
    bool found = find_module_file(module_name, &filename);

    if (!found)
        return name_FileNotFound;

    Term* import = load_module_from_file(module_name, as_cstring(&filename))->owningTerm;

    // If a loadCall is provided, possibly move the new import to be before the loadCall.
    if (loadCall != NULL) {
        Term* callersModule = find_parent_term_in_branch(loadCall, import->owningBranch);

        if (callersModule != NULL && (import->index > callersModule->index))
            move_before(import, callersModule);
    }

    // If the module has static errors, print them now.
    print_static_errors_formatted(nested_contents(import));

    return name_Success;
}

Branch* find_module_from_filename(const char* filename)
{
    // O(n) search for a module with this filename. Could stand to be more efficient.
    for (int i=0; i < kernel()->length(); i++) {
        Term* term = kernel()->get(i);
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

} // namespace circa

using namespace circa;

// Public API
extern "C" {

void circa_run_module(caStack* stack, const char* moduleName)
{
    circa::Branch* branch = nested_contents(get_global(moduleName));

    evaluate_branch((circa::EvalContext*) stack, branch);
}

void circa_add_module_search_path(caWorld* world, const char* path)
{
    modules_add_search_path(path);
}

caBranch* circa_load_module_from_file(caWorld*, const char* module_name, const char* filename)
{
    return (caBranch*) load_module_from_file(module_name, filename);
}

void circa_refresh_module(caBranch* branch)
{
    refresh_script((Branch*) branch);
}

} // extern "C"
