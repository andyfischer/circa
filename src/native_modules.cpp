// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <dlfcn.h>
#include <vector>
#include <map>

#include "block.h"
#include "file.h"
#include "function.h"
#include "hashtable.h"
#include "importing.h"
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
    set_hashtable(&world->everyPatchedFunction);
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

NativeModule* get_existing_native_module(World* world, const char* name)
{
    NativeModuleWorld* nativeWorld = world->nativeModuleWorld;

    // Return existing module, if it exists.
    std::map<std::string, NativeModule*>::const_iterator it =
        nativeWorld->nativeModules.find(name);

    if (it != nativeWorld->nativeModules.end())
        return it->second;

    return NULL;
}

NativeModule* add_native_module(World* world, const char* name)
{
    NativeModuleWorld* nativeWorld = world->nativeModuleWorld;

    NativeModule* module = get_existing_native_module(world, name);

    if (module != NULL)
        return module;

    // Create new module with this name.
    module = create_native_module(world);
    set_string(&module->name, name);
    nativeWorld->nativeModules[name] = module;
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

void native_module_apply_patch(NativeModule* module, Block* block)
{
    // Walk through list of patches, and try to find any functions to apply them to.
    std::map<std::string, EvaluateFunc>::const_iterator it;
    for (it = module->patches.begin(); it != module->patches.end(); ++it) {
        std::string const& name = it->first;
        EvaluateFunc evaluateFunc = it->second;

        Term* term = find_local_name(block, name.c_str());

        if (term == NULL)
            continue;
        if (!is_function(term))
            continue;

        install_function(term, evaluateFunc);
    }
}

void native_module_add_change_action_patch_block(NativeModule* module, const char* blockName)
{
    Value action;
    set_list(&action, 2);
    set_name(list_get(&action, 0), name_PatchBlock);
    set_string(list_get(&action, 1), blockName);

    if (!list_contains(&module->onChangeActions, &action))
        move(&action, list_append(&module->onChangeActions));
}

static void update_patch_function_lookup_for_module(NativeModule* module)
{
    World* world = module->world;
    caValue* everyPatchedFunction = &world->nativeModuleWorld->everyPatchedFunction;

    // std::cout << "looking at module: " << to_string(&module->name) << std::endl;

    // Look across every change action on the NativeModule (looking for PatchBlock entries).
    for (int i=0; i < list_length(&module->onChangeActions); i++) {

        caValue* action = list_get(&module->onChangeActions, i);

        // std::cout << "looking at action: " << to_string(action) << std::endl;

        if (leading_name(action) != name_PatchBlock)
            continue;

        caValue* blockName = list_get(action, 1);

        // Look at every function that this module patches.
        std::map<std::string, EvaluateFunc>::const_iterator it;

        for (it = module->patches.begin(); it != module->patches.end(); it++) {

            Value functionName;
            set_string(&functionName, it->first.c_str());

            // std::cout << "looking at function: " << to_string(&functionName) << std::endl;

            // Construct a global name for this function, using the block's global name.

            Value globalName;
            copy(blockName, &globalName);
            string_append_qualified_name(&globalName, &functionName);

            // Save this line.
            caValue* entry = hashtable_insert(everyPatchedFunction, &globalName);
            set_list(entry, 2);
            copy(&module->name, list_get(entry, 0));
            copy(&functionName, list_get(entry, 1));
        }
    }
}

static void update_patch_function_lookup(World* world)
{
    NativeModuleWorld* nativeWorld = world->nativeModuleWorld;
    set_hashtable(&nativeWorld->everyPatchedFunction);

    // For every native module.
    std::map<std::string, NativeModule*>::const_iterator it;
    for (it = nativeWorld->nativeModules.begin(); it != nativeWorld->nativeModules.end(); ++it) {

        NativeModule* module = it->second;

        update_patch_function_lookup_for_module(module);
    }
}

void native_module_finish_change(NativeModule* module)
{
    World* world = module->world;

    update_patch_function_lookup(world);

    // Run each change action.
    for (int i=0; i < list_length(&module->onChangeActions); i++) {
        caValue* action = list_get(&module->onChangeActions, i);

        Name tag = leading_name(action);

        switch (tag) {
        case name_PatchBlock: {
            caValue* name = list_get(action, 1);
            Block* block = nested_contents(find_from_global_name(world, as_cstring(name)));
            if (block == NULL) {
                // It's okay if the block doesn't exist yet.
                break;
            }

            native_module_apply_patch(module, block);
            break;
        }
        default:
            internal_error("unrecognized action in native_module_finish_change");
        }
    }
}

void module_possibly_patch_new_function(World* world, Block* function)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    Value globalName;
    get_global_name(function, &globalName);

    // No global name: no patch.
    if (!is_string(&globalName))
        return;

    // Lookup in the global table.
    caValue* patchEntry = hashtable_get(&moduleWorld->everyPatchedFunction, &globalName);

    if (patchEntry == NULL) {
        // No patch for this function.
        return;
    }

    caValue* nativeModuleName = list_get(patchEntry, 0);
    caValue* functionName = list_get(patchEntry, 1);

    // Found a patch; apply it.
    NativeModule* module = get_existing_native_module(world, as_cstring(nativeModuleName));

    if (module == NULL) {
        std::cout << "in module_possibly_patch_new_function, couldn't find module: "
            << as_cstring(nativeModuleName) << std::endl;
        return;
    }

    std::map<std::string, EvaluateFunc>::const_iterator it;
    it = module->patches.find(as_cstring(functionName));

    if (it == module->patches.end()) {
        std::cout << "in module_possibly_patch_new_function, couldn't find function: "
            << as_cstring(functionName) << std::endl;
        return;
    }
    
    EvaluateFunc evaluateFunc = it->second;

    install_function(function->owningTerm, evaluateFunc);
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
        std::cout << "dlopen failure, file = " << filename << ", msg = " << dlerror() << std::endl;
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
