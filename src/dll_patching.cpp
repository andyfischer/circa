// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <set>
#include <dlfcn.h>

#include "branch_iterator.h"
#include "dll_patching.h"
#include "function.h"
#include "importing.h"
#include "ref_list.h"

namespace circa {

struct Dll
{
    void* module;

    RefList affectedTerms;
    std::set<void*> loadedFunctions;
};

typedef std::map<std::string, Dll*> LoadedDllMap;
LoadedDllMap loaded_dlls;

void unload_dll(const char* filename)
{
    LoadedDllMap::iterator it = loaded_dlls.find(filename);
    if (it == loaded_dlls.end())
        return;

    Dll* dll = it->second;
    loaded_dlls.erase(it);

    // Track down anyone using this dll, and blank out those function pointers
    for (int i=0; dll->affectedTerms.length(); i++) {
        Term* ref = dll->affectedTerms[i];

        if (!is_function(ref))
            continue;

        FunctionAttrs* attrs = get_function_attrs(ref);

        if (dll->loadedFunctions.find((void*) attrs->evaluate) != dll->loadedFunctions.end())
            attrs->evaluate = empty_evaluate;
    }

    // Platform specific
    dlclose(dll->module);

    delete dll;
}

Dll* load_dll(const char* filename)
{
    Dll* dll = new Dll();

    // Platform specific
    dll->module = dlopen(filename, RTLD_NOW);

    if (dll->module == NULL) {
        delete dll;
        return NULL;
    }

    ca_assert(loaded_dlls.find(filename) == loaded_dlls.end());

    loaded_dlls[filename] = dll;

    return dll;
}

void* find_func_in_dll(Dll* dll, const char* funcName)
{
    return dlsym(dll->module, funcName);
}

void patch_with_dll(const char* dll_filename, Branch& branch)
{
    // Check to unload this file, if it's already loaded
    unload_dll(dll_filename);

    Dll* dll = load_dll(dll_filename);

    // Iterate through every function inside 'branch', and possibly replace
    // its evaluate function with one from the dll.
    for (BranchIterator it(branch); it.unfinished(); it.advance())
    {
        Term* term = *it;
        if (!is_function(term))
            continue;

        FunctionAttrs* attrs = get_function_attrs(term);

        void* newEvaluateFunc = find_func_in_dll(dll, term->name.c_str());

        // Patch in this function and record the affected term
        if (newEvaluateFunc != NULL) {
            attrs->evaluate = (EvaluateFunc) newEvaluateFunc;
            dll->affectedTerms.append(term);
            dll->loadedFunctions.insert(newEvaluateFunc);
        }
    }
}

} // namespace circa
