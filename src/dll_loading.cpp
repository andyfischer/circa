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
#include "static_checking.h"
#include "string_type.h"
#include "names.h"
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

#if WINDOWS
const char* DLL_SUFFIX = ".dll";
#else
const char* DLL_SUFFIX = ".so";
#endif

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

Dll* load_dll(const char* filename, TValue* errorOut)
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

void patch_with_dll(const char* dll_filename, Branch* branch, TValue* errorOut)
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

void find_dll_for_script(Branch* branch, TValue* resultOut)
{
    String filename;
    branch_get_source_filename(branch, &filename);

    if (!is_string(&filename)) {
        set_name(resultOut, name_Failure);
        return;
    }

    String dllFilename;
    copy(&filename, &dllFilename);

    if (string_ends_with(&dllFilename, ".ca"))
        string_resize(&dllFilename, -3);

    //string_append(&dllFilename, DLL_SUFFIX);
    swap(&dllFilename, resultOut);
}

void dll_loading_check_for_patches_on_loaded_branch(Branch* branch)
{
    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        if (it.current()->function == FUNCS.dll_patch) {
            Term* caller = it.current();

            // Find the DLL.
            String filename;
            find_dll_for_script(branch, &filename);

            if (!is_string(&filename)) {
                mark_static_error(caller, "Couldn't find DLL");
                continue;
            }

            TValue error;
            patch_with_dll(as_cstring(&filename), branch, &error);

            if (!is_null(&error)) {
                mark_static_error(caller, &error);
                continue;
            }
        }
    }
}

} // namespace circa
