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
    // Map of names to NativeModules.
    std::map<std::string, NativeModule*> nativeModules;

    // Map of every function's global name -> native module name.
    Value everyPatchedFunction;
};

struct NativeModule
{
    World* world;

    // Module name, unique across the world. Usually the filename.
    Value name;

    std::map<std::string, EvaluateFunc> patches;

    // List of actions that are triggered when this module is changed.
    Value onChangeActions;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};

NativeModuleWorld* create_native_module_world()
{
    NativeModuleWorld* world = new NativeModuleWorld();
    set_dict(&world->everyPatchedFunction);
    return world;
}

NativeModule* create_native_module(World* world)
{
    NativeModule* module = new NativeModule();
    module->world = world;
    module->dll = NULL;
    set_list(&module->onChangeActions, 0);
    return module;
}

void free_native_module(NativeModule* module)
{
    delete module;
}

NativeModule* add_native_module(World* world, const char* name)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    // Return existing module, if it exists.
    std::map<std::string, NativeModule*>::const_iterator it =
        moduleWorld->nativeModules.find(name);

    if (it != moduleWorld->nativeModules.end())
        return it->second;

    // Create module.
    NativeModule* module = create_native_module(world);
    moduleWorld->nativeModules[name] = module;
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

void finish_building_native_module(NativeModule* module)
{
    // Rebuild the everyPatchedFunction dict.
#if 0
    NativeModuleWorld* world = module->world->nativeModuleWorld;

    set_dict(&world->everyPatchedFunction);

    // For every native module.
    std::map<std::string, NativeModule*>::const_iterator it;
    for (it = world->nativeModules.begin(); it != world->nativeModules.end(); ++it) {

        NativeModule* module = it->second;

        // For every branch that this native module affects.
        for (int affectsBranchIndex=0;
                affectsBranchIndex < list_length(&module->affectsBranches);
                affectsBranchIndex++) {

            // For every function patched by this module.
            std::map<std::string, EvaluateFunc>::const_iterator patchIt;

            for (patchIt = module->patches.begin(); patchIt != module->patches.end(); patchIt++) {

                Value functionName;
                set_string(&functionName, patchIt->first.c_str());

                Value globalName;
                copy(list_get(&module->affectsBranches, affectsBranchIndex), &globalName);

                string_append_qualified_name(&globalName, &functionName);

                // TODO
            }
        }
    }
#endif
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

void native_module_add_change_action_patch_branch(NativeModule* module, const char* branchName)
{
    Value action;
    set_list(&action, 2);
    set_name(list_get(&action, 0), name_PatchBranch);
    set_string(list_get(&action, 1), branchName);

    if (!list_contains(&module->onChangeActions, &action))
        move(&action, list_append(&module->onChangeActions));
}

void native_module_on_change(NativeModule* module)
{
    World* world = module->world;

    // Run each change action.
    for (int i=0; i < list_length(&module->onChangeActions); i++) {
        caValue* action = list_get(&module->onChangeActions, i);

        Name tag = leading_name(action);

        switch (tag) {
        case name_PatchBranch: {
            caValue* name = list_get(action, 1);
            Branch* branch = nested_contents(find_from_global_name(world, as_cstring(name)));
            if (branch == NULL) {
                std::cout << "branch " << as_cstring(name)
                    << " not found in native_module_on_change" << std::endl;
                break;
            }

            native_module_apply_patch(module, branch);
            break;
        }
        default:
            internal_error("unrecognized action in native_module_on_change");
        }
    }
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

            Branch* target = NULL; // FIXME

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

    if (!module->dll) {
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
