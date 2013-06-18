// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#if CIRCA_ENABLE_DLL_LOADING

#ifdef _MSC_VER
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

typedef void (*OnLoadFunc)(Block* block);

typedef std::map<std::string, Dll*> LoadedDllMap;
LoadedDllMap g_loadedDlls;

#ifdef _MSC_VER
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

        Function* attrs = as_function(term_value(ref));

        if (dll->loadedFunctions.find((void*) attrs->evaluate) != dll->loadedFunctions.end())
            attrs->evaluate = NULL;
    }

    // Platform specific
    #ifdef _MSC_VER
        // TODO
    #else
        dlclose(dll->module);
    #endif

    delete dll;
}

void* find_func_in_dll(Dll* dll, const char* funcName)
{
    #ifdef _MSC_VER
        //TODO
        return NULL;
    #else
        return dlsym(dll->module, funcName);
    #endif
}

Dll* load_dll(const char* filename, caValue* errorOut)
{
    Dll* dll = new Dll();

    const int actual_filename_max_len = 250;
    char actual_filename[actual_filename_max_len];

    // Platform specific
    #ifdef _MSC_VER
        // TODO
        dll->module = NULL;
    #else
        sprintf(actual_filename, "%s.so", filename);
        dll->module = dlopen(actual_filename, RTLD_NOW);
    #endif

    if (dll->module == NULL) {
        std::string error("dlopen failed to open ");
        error += actual_filename;
        #ifdef _MSC_VER
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

void patch_block_recr(Dll* dll, Block* block, std::string namespacePrefix)
{
    for (int i=0; i < block->length(); i++)
    {
        Term* term = block->get(i);

        if (is_namespace(term)) {
            patch_block_recr(dll, nested_contents(term), namespacePrefix + term->name + "__");
        }
        else if (is_function(term)) {
            std::string name = term->name;

            // Replace '.' with '_'
            for (size_t i=0; i < name.length(); i++)
                if (name[i] == '.')
                    name[i] = '_';

            std::string searchSymbol = namespacePrefix + name;

            void* newEvaluateFunc = find_func_in_dll(dll, searchName.c_str());

            // Patch in this function and record the affected term
            if (newEvaluateFunc != NULL) {
                function_set_evaluate_func(as_function2(term), (EvaluateFunc) newEvaluateFunc);
                dll->affectedTerms.append(term);
                dll->loadedFunctions.insert(newEvaluateFunc);
            }
        }
    }
}

void patch_with_dll(const char* dll_filename, Block* block, caValue* errorOut)
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
        onLoad(block);

    // Iterate through every function inside 'block', and possibly replace
    // its evaluate function with one from the dll.
    patch_block_recr(dll, block, "");
}

void find_dll_for_script(Block* block, caValue* resultOut)
{
    String* filename = (String*) block_get_source_filename(block);

    if (!is_string(filename)) {
        set_symbol(resultOut, sym_Failure);
        return;
    }

    String dllFilename;
    copy(filename, &dllFilename);

    if (string_ends_with(&dllFilename, ".ca"))
        string_resize(&dllFilename, -3);

    //string_append(&dllFilename, DLL_SUFFIX);
    swap(&dllFilename, resultOut);
}

void dll_loading_check_for_patches_on_loaded_block(Block* block)
{
    for (BlockIteratorFlat it(block); it.unfinished(); it.advance()) {
        if (it.current()->function == FUNCS.dll_patch) {
            Term* caller = it.current();

            // Find the DLL.
            String filename;
            find_dll_for_script(block, &filename);

            if (!is_string(&filename)) {
                mark_static_error(caller, "Couldn't find DLL");
                continue;
            }

            Value error;
            patch_with_dll(as_cstring(&filename), block, &error);

            if (!is_null(&error)) {
                std::cout << as_string(&error) << std::endl;

                mark_static_error(caller, &error);
                continue;
            }
        }
    }
}

} // namespace circa

#else // CIRCA_ENABLE_DLL_LOADING

namespace circa {

void dll_loading_check_for_patches_on_loaded_block(Block* block)
{
    // No-op
}

} // namespace circa

#endif // CIRCA_ENABLE_DLL_LOADING
