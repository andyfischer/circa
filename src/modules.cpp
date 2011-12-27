// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "filesystem.h"
#include "list_shared.h"
#include "kernel.h"
#include "modules.h"
#include "string_type.h"
#include "symbols.h"
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
    set_string(g_moduleSearchPaths.append(), str);
}

Term* find_loaded_module(Symbol name)
{
    String nameStr;
    symbol_get_text(name, &nameStr);

    for (BranchIteratorFlat it(kernel()); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (term->function == BUILTIN_FUNCS.imported_file && term->name == as_string(&nameStr))
            return term;
    }
    return NULL;
}

static Term* load_module_from_file(Symbol module_name, const char* filename)
{
    String name;
    symbol_get_text(module_name, &name);

    Term* import = apply(kernel(), BUILTIN_FUNCS.imported_file, TermList(),
        as_cstring(&name));
    load_script(nested_contents(import), filename);

    return import;
}

static bool find_module_file(Symbol module_name, String* filenameOut)
{
    String module;
    symbol_get_text(module_name, &module);

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

Symbol load_module(Symbol module_name, Term* loadCall)
{
    Term* existing = find_loaded_module(module_name);
    if (existing != NULL)
        return Success;
    
    String filename;
    bool found = find_module_file(module_name, &filename);

    if (!found)
        return FileNotFound;

    Term* import = load_module_from_file(module_name, as_cstring(&filename));

    // If a loadCall is provided, possibly move the new import to be before the loadCall.
    if (loadCall != NULL) {
        Term* callersModule = find_parent_term_in_branch(loadCall, import->owningBranch);

        if (callersModule != NULL && (import->index > callersModule->index))
            move_before(import, callersModule);
    }

    return Success;
}

} // namespace circa
