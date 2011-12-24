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
        if (term->function == BUILTIN_FUNCS.import && term->name == as_string(&nameStr))
            return term;
    }
    return NULL;
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
        String searchPath;
        copy(g_moduleSearchPaths[i], &searchPath);

        join_path(&searchPath, &module);
        join_path(&searchPath, &module);
        string_append(&searchPath, ".ca");

        if (file_exists(as_cstring(&searchPath))) {
            Term* import = apply(kernel(), BUILTIN_FUNCS.import, TermList());
            load_script(nested_contents(import), as_cstring(&searchPath));
            return Success;
        }
    }
    return FileNotFound;
}

} // namespace circa
