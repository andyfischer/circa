// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include <set>

#ifdef WINDOWS
// TODO
#else
#include <dlfcn.h>
#endif

#include "code_iterators.h"
#include "function.h"
#include "importing.h"

namespace circa {
namespace dll_loading_function {

    struct Dll
    {
        void* module;

        TermList affectedTerms;
        std::set<void*> loadedFunctions;
    };

    typedef void (*OnLoadFunc)(Branch* branch);

    typedef std::map<std::string, Dll*> LoadedDllMap;
    LoadedDllMap g_loadedDlls;

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

            FunctionAttrs* attrs = get_function_attrs(ref);

            if (dll->loadedFunctions.find((void*) attrs->evaluate) != dll->loadedFunctions.end())
                attrs->evaluate = empty_evaluate_function;
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

    void patch_branch_recr(Dll* dll, Branch& branch, std::string namespacePrefix)
    {
        for (int i=0; i < branch.length(); i++)
        {
            Term* term = branch[i];

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

    void patch_with_dll(const char* dll_filename, Branch& branch, TaggedValue* errorOut)
    {
        // Check to unload this file, if it's already loaded
        unload_dll(dll_filename);

        Dll* dll = load_dll(dll_filename, errorOut);

        if (dll == NULL) {
            if (errorOut != NULL)
                set_string(errorOut, "failed to load DLL");
            return;
        }

        // Call on_load (if it exists)
        OnLoadFunc onLoad = (OnLoadFunc) find_func_in_dll(dll, "on_load");
        if (onLoad != NULL)
            onLoad(&branch);

        // Iterate through every function inside 'branch', and possibly replace
        // its evaluate function with one from the dll.
        patch_branch_recr(dll, branch, "");
    }

    CA_FUNCTION(load_and_patch)
    {
        Branch* branch = as_branch(INPUT(0));
        const char* filename = STRING_INPUT(1);

        TaggedValue error;
        patch_with_dll(filename, *branch, &error);

        if (!is_null(&error))
            error_occurred(CONTEXT, CALLER, as_string(&error));
    }

    CA_FUNCTION(dll_filename)
    {
        std::string base_filename = STRING_INPUT(0);
        set_string(OUTPUT, base_filename + ".so");
    }
    CA_FUNCTION(source_filename)
    {
        std::string base_filename = STRING_INPUT(0);
        set_string(OUTPUT, base_filename + ".cpp");
    }

    CA_FUNCTION(rebuild_dll)
    {
        std::string base_filename = STRING_INPUT(0);
        std::cout << "rebuilding " << base_filename << std::endl;
        std::string dir = get_directory_for_filename(base_filename);

        // TODO: This is probably terrible:
        std::string cmd ="cd " + dir + "; make"; 
        int ret = system(cmd.c_str());
        if (ret != 0)
            return error_occurred(CONTEXT, CALLER, "'make' returned error");
    }

    void setup(Branch& kernel)
    {
        Branch& ns = create_namespace(kernel, "dll_loading");
        import_function(ns, load_and_patch,
                "load_and_patch(Branch branch, string filename)");
        import_function(ns, dll_filename,
                "dll_filename(string baseFilename) -> string");
        import_function(ns, source_filename,
                "source_filename(string baseFilename) -> string");
        import_function(ns, rebuild_dll,
                "rebuild(string baseFilename)");
    }
}
} // namespace circa
