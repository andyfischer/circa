// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "filesystem.h"
#include "list_shared.h"
#include "kernel.h"
#include "modules.h"
#include "names.h"
#include "static_checking.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "types/list.h"

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

Term* find_loaded_module(Name name)
{
    String nameStr;
    name_to_string(name, &nameStr);

    for (BranchIteratorFlat it(kernel()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == FUNCS.imported_file && term->name == as_string(&nameStr))
            return term;
    }
    return NULL;
}

Term* load_module_from_file(Name module_name, const char* filename)
{
    String name;
    name_to_string(module_name, &name);

    Term* import = apply(kernel(), FUNCS.imported_file, TermList(),
        as_cstring(&name));
    load_script(nested_contents(import), filename);

    return import;
}

static bool find_module_file(Name module_name, String* filenameOut)
{
    String module;
    name_to_string(module_name, &module);

    int count = list_length(&g_moduleSearchPaths);
    for (int i=0; i < count; i++) {

        // For each search path we'll check two places.

        // Look under searchPath/moduleName.ca
        String searchPath;
        copy(g_moduleSearchPaths[i], &searchPath);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }

        // Look under searchPath/moduleName/moduleName.ca
        copy(g_moduleSearchPaths[i], &searchPath);

        join_path(&searchPath, &module);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (file_exists(as_cstring(&searchPath))) {
            swap(&searchPath, filenameOut);
            return true;
        }
    }
    return false;
}

Name load_module(Name module_name, Term* loadCall)
{
    Term* existing = find_loaded_module(module_name);
    if (existing != NULL)
        return name_Success;
    
    String filename;
    bool found = find_module_file(module_name, &filename);

    if (!found)
        return name_FileNotFound;

    Term* import = load_module_from_file(module_name, as_cstring(&filename));

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
