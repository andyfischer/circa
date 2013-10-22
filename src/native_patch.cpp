// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#ifndef CIRCA_DISABLE_DLL
  #include <dlfcn.h>
#endif

#include <vector>
#include <map>

#include "block.h"
#include "file.h"
#include "function.h"
#include "hashtable.h"
#include "importing.h"
#include "kernel.h"
#include "names.h"
#include "native_patch.h"
#include "string_type.h"
#include "term.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

struct NativePatchWorld
{
    // Map of names to NativePatchs.
    std::map<std::string, NativePatch*> nativeModules;

    // Map of every function's global name -> native module name.
    Value everyPatchedFunction;
};

struct NativePatch
{
    World* world;

    std::map<std::string, EvaluateFunc> patches;

    // Target block name.
    Value targetName;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};

NativePatchWorld* alloc_native_patch_world()
{
    NativePatchWorld* world = new NativePatchWorld();
    set_hashtable(&world->everyPatchedFunction);
    return world;
}

void free_native_patch_world(NativePatchWorld* world)
{
    std::map<std::string, NativePatch*>::const_iterator it;
    for (it = world->nativeModules.begin(); it != world->nativeModules.end(); ++it) {
        NativePatch* patch = it->second;
        delete patch;
    }
    delete world;
}

NativePatch* alloc_native_patch(World* world)
{
    NativePatch* patch = new NativePatch();
    patch->world = world;
    patch->dll = NULL;
    return patch;
}

void free_native_patch(NativePatch* patch)
{
    delete patch;
}

NativePatch* get_existing_native_patch(World* world, const char* name)
{
    NativePatchWorld* nativeWorld = world->nativePatchWorld;

    // Return existing module, if it exists.
    std::map<std::string, NativePatch*>::const_iterator it =
        nativeWorld->nativeModules.find(name);

    if (it != nativeWorld->nativeModules.end())
        return it->second;

    return NULL;
}

NativePatch* add_native_patch(World* world, const char* targetName)
{
    NativePatchWorld* nativeWorld = world->nativePatchWorld;

    NativePatch* module = get_existing_native_patch(world, targetName);

    if (module != NULL)
        return module;

    // Create new module with this name.
    module = alloc_native_patch(world);
    set_string(&module->targetName, targetName);
    nativeWorld->nativeModules[targetName] = module;
    return module;
}

void remove_native_patch(World* world, const char* name)
{
    world->nativePatchWorld->nativeModules.erase(name);

    std::map<std::string, NativePatch*>::iterator it;
    it = world->nativePatchWorld->nativeModules.find(name);

    if (it != world->nativePatchWorld->nativeModules.end()) {
        free_native_patch(it->second);
        world->nativePatchWorld->nativeModules.erase(it);
    }
}

void module_patch_function(NativePatch* module, const char* name, EvaluateFunc func)
{
    module->patches[name] = func;
}

void native_patch_apply_patch(NativePatch* module, Block* block)
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

static void update_patch_function_lookup_for_module(NativePatch* module)
{
    World* world = module->world;
    caValue* everyPatchedFunction = &world->nativePatchWorld->everyPatchedFunction;

    caValue* targetName = &module->targetName;

    // Look at every function that this module patches.
    std::map<std::string, EvaluateFunc>::const_iterator it;

    for (it = module->patches.begin(); it != module->patches.end(); it++) {

        Value functionName;
        set_string(&functionName, it->first.c_str());

        // Construct a global name for this function, using the block's global name.

        Value globalName;
        copy(targetName, &globalName);
        string_append_qualified_name(&globalName, &functionName);

        // Save this line.
        caValue* entry = hashtable_insert(everyPatchedFunction, &globalName);
        set_list(entry, 2);
        copy(&module->targetName, list_get(entry, 0));
        copy(&functionName, list_get(entry, 1));
    }
}

static void update_patch_function_lookup(World* world)
{
    NativePatchWorld* nativeWorld = world->nativePatchWorld;
    set_hashtable(&nativeWorld->everyPatchedFunction);

    // For every native module.
    std::map<std::string, NativePatch*>::const_iterator it;
    for (it = nativeWorld->nativeModules.begin(); it != nativeWorld->nativeModules.end(); ++it) {

        NativePatch* module = it->second;

        update_patch_function_lookup_for_module(module);
    }
}

void native_patch_finish_change(NativePatch* module)
{
    World* world = module->world;

    update_patch_function_lookup(world);

    // Apply changes to the target module.
    caValue* targetName = &module->targetName;
    Block* block = nested_contents(find_from_global_name(world, as_cstring(targetName)));
    if (block == NULL)
        // It's okay if the block doesn't exist yet.
        return;

    native_patch_apply_patch(module, block);
}

CIRCA_EXPORT void circa_finish_native_patch(caNativePatch* module)
{
    native_patch_finish_change(module);
}

void module_possibly_patch_new_function(World* world, Block* function)
{
    NativePatchWorld* moduleWorld = world->nativePatchWorld;

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
    NativePatch* module = get_existing_native_patch(world, as_cstring(nativeModuleName));

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

void native_patch_add_platform_specific_suffix(caValue* filename)
{
    string_append(filename, ".so");
}

void native_patch_close(NativePatch* module)
{
#ifdef CIRCA_DISABLE_DLL
    internal_error("native_patch_close failed, DLL support compiled out");
#else
    if (module->dll != NULL)
        dlclose(module->dll);

    module->dll = NULL;
#endif
}

void native_patch_load_from_file(NativePatch* module, const char* filename)
{
#ifdef CIRCA_DISABLE_DLL
    internal_error("native_patch_load_from_file failed, DLL support compiled out");
#else
    native_patch_close(module);

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
#endif
}

CIRCA_EXPORT caNativePatch* circa_create_native_patch(caWorld* world, const char* name)
{
    return add_native_patch(world, name);
}

CIRCA_EXPORT void circa_patch_function(caNativePatch* module, const char* name,
        caEvaluateFunc func)
{
    module_patch_function((NativePatch*) module, name, func);
}

} // namespace circa
