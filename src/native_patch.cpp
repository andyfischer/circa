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
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "native_patch.h"
#include "string_type.h"
#include "term.h"
#include "type.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

typedef std::map<std::string, NativePatch*> StringToNativePatchMap;

struct NativePatchWorld
{
    // Map of names to NativePatches.
    StringToNativePatchMap nativeModules;
};

struct NativePatch
{
    World* world;

    // Hashtable, maps names to patches.
    Value patches;

    // Target block name.
    Value targetName;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};

NativePatchWorld* alloc_native_patch_world()
{
    NativePatchWorld* world = new NativePatchWorld();
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
    set_hashtable(&patch->patches);
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

NativePatch* insert_native_patch(World* world, const char* targetName)
{
    NativePatchWorld* nativeWorld = world->nativePatchWorld;

    NativePatch* module = get_existing_native_patch(world, targetName);

    if (module != NULL)
        return module;

    // Create new NativePatch with this name.
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

static void function_key(const char* name, caValue* out)
{
    set_list(out, 2);
    set_symbol(out->index(0), sym_Function);
    set_string(out->index(1), name);
}

static void type_release_key(const char* typeName, caValue* out)
{
    set_list(out, 2);
    set_symbol(list_get(out, 0), sym_TypeRelease);
    set_string(list_get(out, 1), typeName);
}

void module_patch_function(NativePatch* module, const char* name, EvaluateFunc func)
{
    Value key;
    function_key(name, &key);
    set_opaque_pointer(hashtable_insert(&module->patches, &key), (void*) func);
}

void module_patch_type_release(NativePatch* module, const char* typeName, ReleaseFunc func)
{
    Value key;
    type_release_key(typeName, &key);
    set_opaque_pointer(hashtable_insert(&module->patches, &key), (void*) func);
}

void native_patch_apply_patch(NativePatch* module, Block* block)
{
    // Walk through list of patches, and apply them to block terms as appropriate.
    
    for (HashtableIterator it(&module->patches); it; ++it) {
        caValue* patchType = it.key()->index(0);
        caValue* name = it.key()->index(1);

        Term* term = find_local_name(block, name);

        if (term == NULL)
            continue;

        switch (as_symbol(patchType)) {
        case sym_Function: {
            if (!is_function(term))
                break;

            EvaluateFunc func = (EvaluateFunc) as_opaque_pointer(it.value());
            install_function(term, func);
            break;
        }

        case sym_TypeRelease: {
            if (!is_type(term))
                break;

            ReleaseFunc func = (ReleaseFunc) as_opaque_pointer(it.value());
            as_type(term_value(term))->release = func;
            break;
        }
        }
    }
}

void native_patch_finish_change(NativePatch* module)
{
    World* world = module->world;

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

static NativePatch* find_native_patch_for_module(NativePatchWorld* world, Block* module)
{
    if (module == NULL)
        return NULL;

    StringToNativePatchMap::const_iterator it = world->nativeModules.find(
        as_cstring(term_name(module->owningTerm)));

    if (it == world->nativeModules.end())
        return NULL;

    return it->second;
}

static bool term_can_be_patched(Term* term)
{
    // Currently, only functions & types at the module level can be patched.
    return is_module(term->owningBlock);
}

Block* find_enclosing_module(Block* block)
{
    while (true) {
        if (block == NULL)
            return NULL;
        if (is_module(block))
            return block;
        block = get_parent_block(block);
    }
}

void native_patch_apply_to_new_function(World* world, Block* function)
{
    Term* term = function->owningTerm;
    if (!term_can_be_patched(term))
        return;

    NativePatchWorld* moduleWorld = world->nativePatchWorld;

    Block* module = find_enclosing_module(function);
    if (module == NULL)
        return;

    NativePatch* nativePatch = find_native_patch_for_module(moduleWorld, module);
    if (nativePatch == NULL)
        return;

    Value key;
    function_key(as_cstring(term_name(term)), &key);
    caValue* patch = hashtable_get(&nativePatch->patches, &key);
    if (patch == NULL)
        return;
        
    EvaluateFunc evaluateFunc = (EvaluateFunc) as_opaque_pointer(patch);
    install_function(term, evaluateFunc);
}

void native_patch_apply_to_new_type(World* world, Type* type)
{
    Term* term = type->declaringTerm;
    if (!term_can_be_patched(term))
        return;

    NativePatchWorld* moduleWorld = world->nativePatchWorld;

    Block* module = find_enclosing_module(term->owningBlock);
    if (module == NULL)
        return;

    NativePatch* nativePatch = find_native_patch_for_module(moduleWorld, module);
    if (nativePatch == NULL)
        return;

    Value key;
    type_release_key(as_cstring(term_name(term)), &key);
    caValue* patch = hashtable_get(&nativePatch->patches, &key);
    if (patch == NULL)
        return;
        
    ReleaseFunc releaseFunc = (ReleaseFunc) as_opaque_pointer(patch);
    type->release = releaseFunc;
}

void native_patch_add_platform_specific_suffix(caValue* filename)
{
    // Future: Windows support.
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
    return insert_native_patch(world, name);
}

CIRCA_EXPORT void circa_patch_function(caNativePatch* module, const char* name,
        caEvaluateFunc func)
{
    module_patch_function((NativePatch*) module, name, func);
}

} // namespace circa
