// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <dlfcn.h>
#include <vector>
#include <map>

#include "branch.h"
#include "file.h"
#include "function.h"
#include "kernel.h"
#include "names.h"
#include "native_modules.h"
#include "string_type.h"
#include "term.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

struct NativeModuleWorld
{
    std::map<std::string, NativeModule*> nativeModules;
};

struct NativeModule
{
    std::map<std::string, EvaluateFunc> patches;

    Value namePrefix;

    OnModuleLoad onModuleLoad;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};

NativeModuleWorld* create_native_module_world()
{
    return new NativeModuleWorld();
}

NativeModule* create_native_module()
{
    NativeModule* module = new NativeModule();
    module->dll = NULL;
    return module;
}

void free_native_module(NativeModule* module)
{
    delete module;
}

NativeModule* add_native_module(World* world, const char* namePrefix)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    // Return existing module, if it exists.
    std::map<std::string, NativeModule*>::const_iterator it =
        moduleWorld->nativeModules.find(namePrefix);

    if (it != moduleWorld->nativeModules.end())
        return it->second;

    // Create module.
    NativeModule* module = create_native_module();
    moduleWorld->nativeModules[namePrefix] = module;
    set_string(&module->namePrefix, namePrefix);
    return module;
}

void delete_native_module(World* world, const char* name)
{
    world->nativeModuleWorld->nativeModules.erase(name);
}

void module_patch_function(NativeModule* module, const char* name, EvaluateFunc func)
{
    module->patches[name] = func;
}    

void native_module_apply_patch(NativeModule* module, Branch* branch)
{
    bool anyTouched = false;

    // Walk through list of patches, and try to find any functions to apply them to.
    std::map<std::string, EvaluateFunc>::const_iterator it;
    for (it = module->patches.begin(); it != module->patches.end(); ++it) {
        std::string const& name = it->first;
        EvaluateFunc evaluateFunc = it->second;

        Term* term = find_local_name(branch, name.c_str());

        if (term == NULL)
            continue;
        if (!is_function(term))
            continue;

        Function* function = as_function(term);
        function->evaluate = evaluateFunc;
        anyTouched = true;
        dirty_bytecode(function_contents(term));
    }

    if (anyTouched)
        dirty_bytecode(branch);
}

Branch* native_module_get_target_branch(World* world, NativeModule* module)
{
    if (!is_string(&module->namePrefix))
        return world->root;

    Term* term = find_from_global_name(world, as_cstring(&module->namePrefix));
    if (term == NULL)
        return NULL;

    return term->nestedContents;
}

void module_on_loaded_branch(Branch* branch)
{
#if 0
    // Search the script for calls to native_patch.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term->function != FUNCS.native_patch)
            continue;

        // Load the native module.
        Value filename;
        get_path_relative_to_source(branch, term_value(term->input(0)), &filename);

        Value fileWatchAction;
        // ...
    }
#endif
}

void module_possibly_patch_new_function(World* world, Branch* function)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    Value functionGlobalName;
    get_global_name(function, &functionGlobalName);

    // No global name: no patch.
    if (!is_string(&functionGlobalName))
        return;

    // Check every loaded native module; see if our function's global name starts
    // with any of the module names.

    std::map<std::string, NativeModule*>::const_iterator it;
    for (it = moduleWorld->nativeModules.begin(); it != moduleWorld->nativeModules.end(); ++it) {
        if (string_starts_with(&functionGlobalName, it->first.c_str())) {

            NativeModule* module = it->second;

            Branch* target = native_module_get_target_branch(world, module);

            if (target == NULL)
                continue;

            // Check inside this module.
            std::map<std::string, EvaluateFunc>::const_iterator patchIt;
            for (patchIt = module->patches.begin(); patchIt != module->patches.end(); ++patchIt) {

                Term* term = find_local_name(target, patchIt->first.c_str());
                if (term->nestedContents == function) {
                    // Found a match.
                    EvaluateFunc evaluateFunc = patchIt->second;
                    as_function(term)->evaluate = evaluateFunc;
                    dirty_bytecode(function);
                }
            }
        }
    }
}

void native_module_add_platform_specific_suffix(caValue* filename)
{
    string_append(filename, ".so");
}

void native_module_close(NativeModule* module)
{
    if (module->dll != NULL)
        dlclose(module->dll);

    module->dll = NULL;
}

void native_module_load_from_file(NativeModule* module, const char* filename)
{
    native_module_close(module);

    module->dll = dlopen(filename, RTLD_NOW);

    if (module->dll) {
        std::cout << "failed to open dll: " << filename << std::endl;
        return;
    }

    // Future: check the dll's reported API version.

    OnModuleLoad moduleLoad = (OnModuleLoad) dlsym(module->dll, "circa_module_load");

    if (moduleLoad == NULL) {
        std::cout << "could not find circa_module_load in: " << filename << std::endl;
        return;
    }

    moduleLoad(module);
}

CIRCA_EXPORT void circa_patch_function(caNativeModule* module, const char* name,
        caEvaluateFunc func)
{
    module_patch_function((NativeModule*) module, name, func);
}

} // namespace circa
