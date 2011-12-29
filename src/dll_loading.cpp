// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#ifdef WINDOWS
#else
#include <dlfcn.h>
#endif

#include <set>

#include "code_iterators.h"
#include "function.h"
#include "kernel.h"
#include "string_type.h"
#include "symbols.h"
#include "term.h"

namespace circa {

struct Dll
{
    void* module;

    TermList affectedTerms;
    std::set<void*> loadedFunctions;
};

typedef void (*OnLoadFunc)(Branch* branch);

typedef std::map<std::string, Dll*> LoadedDllMap;
LoadedDllMap g_loadedDlls;

void* find_func_in_dll(Dll* dll, const char* funcName);

void unload_dll(const char* filename)
{
    LoadedDllMap::iterator it = g_loadedDlls.find(filename);
    if (it == g_loadedDlls.end())
        return;

    Dll* dll = it->second;
    g_loadedDlls.erase(it);

    // Track down anyone using this dll, and blank out those function pointers
    for (int i=0; i < dll->affectedTerms.length(); i++) {
        Term* ref = dll->affectedTerms[i];

        if (!is_function(ref))
            continue;

        Function* attrs = as_function(ref);

        if (dll->loadedFunctions.find((void*) attrs->evaluate) != dll->loadedFunctions.end())
            attrs->evaluate = NULL;
    }

    // Platform specific
    #ifdef WINDOWS
        // TODO
    #else
        dlclose(dll->module);
    #endif

    delete dll;
}

void* find_func_in_dll(Dll* dll, const char* funcName)
{
    #ifdef WINDOWS
        //TODO
        return NULL;
    #else
        return dlsym(dll->module, funcName);
    #endif
}

Dll* load_dll(const char* filename, TaggedValue* errorOut)
{
    Dll* dll = new Dll();

    const int actual_filename_max_len = 250;
    char actual_filename[actual_filename_max_len];

    // Platform specific
    #ifdef WINDOWS
        // TODO
        dll->module = NULL;
    #else
        sprintf(actual_filename, "%s.so", filename);
        dll->module = dlopen(actual_filename, RTLD_NOW);
    #endif

    if (dll->module == NULL) {
        std::string error("dlopen failed to open ");
        error += actual_filename;
        #ifdef WINDOWS
            // TODO
        #else
            error += std::string(": \n") + dlerror();
        #endif

        set_string(errorOut, error);
        delete dll;
        return NULL;
    }

    ca_assert(g_loadedDlls.find(filename) == g_loadedDlls.end());

    g_loadedDlls[filename] = dll;

    return dll;
}

void patch_branch_recr(Dll* dll, Branch* branch, std::string namespacePrefix)
{
    for (int i=0; i < branch->length(); i++)
    {
        Term* term = branch->get(i);

        if (is_namespace(term)) {
            patch_branch_recr(dll, nested_contents(term), namespacePrefix + term->name + "__");
        }
        else if (is_function(term)) {
            std::string name = term->name;

            // Replace '.' with '_'
            for (size_t i=0; i < name.length(); i++)
                if (name[i] == '.')
                    name[i] = '_';

            std::string searchName = namespacePrefix + name;

            void* newEvaluateFunc = find_func_in_dll(dll, searchName.c_str());

            // Patch in this function and record the affected term
            if (newEvaluateFunc != NULL) {
                function_set_evaluate_func(term, (EvaluateFunc) newEvaluateFunc);
                dll->affectedTerms.append(term);
                dll->loadedFunctions.insert(newEvaluateFunc);
            }
        }
    }
}

void patch_with_dll(const char* dll_filename, Branch* branch, TaggedValue* errorOut)
{
    // Check to unload this file, if it's already loaded
    unload_dll(dll_filename);

    Dll* dll = load_dll(dll_filename, errorOut);

    if (dll == NULL) {
        if (errorOut != NULL) {
            std::stringstream msg;
            msg << "load_dll failed, " << as_string(errorOut);
            set_string(errorOut, msg.str().c_str());
        }
        return;
    }

    // Call on_load (if it exists)
    OnLoadFunc onLoad = (OnLoadFunc) find_func_in_dll(dll, "on_load");
    if (onLoad != NULL)
        onLoad(branch);

    // Iterate through every function inside 'branch', and possibly replace
    // its evaluate function with one from the dll.
    patch_branch_recr(dll, branch, "");
}

void find_dll_for_script(Branch* branch, TaggedValue* resultOut)
{
    String filename;
    branch_get_source_filename(branch, &filename);

    if (!is_string(&filename)) {
        set_symbol(resultOut, Failure);
        return;
    }

    if (string_ends_with(&filename, ".ca")) {
        // ...
    }
}

void dll_loading_check_for_patches_on_loaded_branch(Branch* branch)
{
    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        if (it.current()->function == BUILTIN_FUNCS.dll_patch) {
            Term* caller = it.current();

            // Find the DLL.


            //dll_loading_patch_dll_default(branch);
            return;
        }
    }
}

} // namespace circa
