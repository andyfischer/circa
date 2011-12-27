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
List g_loadedModules;

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

static Symbol load_module_from_file(Symbol module_name, const char* filename)
{
    String name;
    symbol_get_text(module_name, &name);

    Term* import = apply(kernel(), BUILTIN_FUNCS.imported_file, TermList(),
        as_cstring(&name));
    load_script(nested_contents(import), filename);

    return Success;
}

Symbol load_module(Symbol module_name)
{
    Term* existing = find_loaded_module(module_name);
    if (existing != NULL)
        return Success;
    
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

        if (file_exists(as_cstring(&searchPath)))
            return load_module_from_file(module_name, as_cstring(&searchPath));

        // Look under searchPath/moduleName/moduleName.ca
        copy(g_moduleSearchPaths[i], &searchPath);

        join_path(&searchPath, &module);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (file_exists(as_cstring(&searchPath)))
            return load_module_from_file(module_name, as_cstring(&searchPath));
    }
    return FileNotFound;
}

} // namespace circa
