// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

typedef void (*OnLoadFunc)(Branch* branch);

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
    for (int i=0; i < dll->affectedTerms.length(); i++) {
        Term* ref = dll->affectedTerms[i];

        if (!is_function(ref))
            continue;

        FunctionAttrs* attrs = get_function_attrs(ref);

        if (dll->loadedFunctions.find((void*) attrs->evaluate) != dll->loadedFunctions.end())
            attrs->evaluate = empty_evaluate_function;
    }

    // Platform specific
    dlclose(dll->module);

    delete dll;
}

void* find_func_in_dll(Dll* dll, const char* funcName)
{
    return dlsym(dll->module, funcName);
}

Dll* load_dll(const char* filename, TaggedValue* errorOut)
{
    Dll* dll = new Dll();

    const int actual_filename_max_len = 250;
    char actual_filename[actual_filename_max_len];

    // Platform specific
    sprintf(actual_filename, "%s.so", filename);
    dll->module = dlopen(actual_filename, RTLD_NOW);

    if (dll->module == NULL) {
        set_string(errorOut, std::string("dlopen failed to open ")+actual_filename
                +": \n"+dlerror());
        delete dll;
        return NULL;
    }

    ca_assert(loaded_dlls.find(filename) == loaded_dlls.end());

    loaded_dlls[filename] = dll;

    return dll;
}


void patch_branch_recr(Dll* dll, Branch& branch, std::string namespacePrefix)
{
    for (int i=0; i < branch.length(); i++)
    {
        Term* term = branch[i];

        if (is_namespace(term)) {
            patch_branch_recr(dll, term->nestedContents, namespacePrefix + term->name + "__");
        }
        else if (is_function(term)) {
            FunctionAttrs* attrs = get_function_attrs(term);

            std::string searchName = namespacePrefix + term->name.c_str();

            void* newEvaluateFunc = find_func_in_dll(dll, searchName.c_str());

            // Patch in this function and record the affected term
            if (newEvaluateFunc != NULL) {
                attrs->evaluate = (EvaluateFunc) newEvaluateFunc;
                dll->affectedTerms.append(term);
                dll->loadedFunctions.insert(newEvaluateFunc);
            }
        }
    }
}

void patch_with_dll(const char* dll_filename, Branch& branch, TaggedValue* errorOut)
{
    // Check to unload this file, if it's already loaded
    unload_dll(dll_filename);

    Dll* dll = load_dll(dll_filename, errorOut);

    if (dll == NULL)
        return;

    // Call on_load (if it exists)
    OnLoadFunc onLoad = (OnLoadFunc) find_func_in_dll(dll, "on_load");
    if (onLoad != NULL)
        onLoad(&branch);

    // Iterate through every function inside 'branch', and possibly replace
    // its evaluate function with one from the dll.
    patch_branch_recr(dll, branch, "");
}

} // namespace circa
