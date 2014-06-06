// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#define NATIVE_PATCH_VERBOSE 0

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

struct NativeFunc
{
    Value name;
    EvaluateFunc func;
};

struct NativePatch
{
    // One "NativePatch" is a collection of name and native-function pairs that all apply
    // to a single module.
    
    World* world;

    // Hashtable, maps names to funcTable index.
    Value funcMap;

    // Target module name.
    Value name;

    // If this module was loaded from a DLL or shared object, that object is here.
    // May be NULL if the module was created a different way.
    void* dll;
};


static int add_native_func(World* world, Value* name, EvaluateFunc func)
{
    int index = world->funcTableCount;
    world->funcTableCount++;
    world->funcTable = (NativeFunc*) realloc(world->funcTable, sizeof(NativeFunc) * world->funcTableCount);
    NativeFunc* newFunc = &world->funcTable[world->funcTableCount - 1];
    initialize_null(&newFunc->name);
    copy(name, &newFunc->name);
    newFunc->func = func;
    return index;
}

static NativePatch* add_native_patch(World* world)
{
    world->nativePatchCount++;
    world->nativePatch = (NativePatch*) realloc(world->nativePatch,
        sizeof(NativePatch) * world->nativePatchCount);
    
    NativePatch* patch = &world->nativePatch[world->nativePatchCount - 1];
    initialize_null(&patch->funcMap);
    initialize_null(&patch->name);
    set_hashtable(&patch->funcMap);
    patch->world = world;
    patch->dll = NULL;
    return patch;
}

void free_native_patch(NativePatch* patch)
{
    delete patch;
}

NativePatch* find_existing_native_patch(World* world, const char* name)
{
    for (int i=0; i < world->nativePatchCount; i++) {
        NativePatch* patch = &world->nativePatch[i];
        if (string_equals(&patch->name, name))
            return patch;
    }

    return NULL;
}

NativePatch* insert_native_patch(World* world, const char* name)
{
    NativePatch* patch = find_existing_native_patch(world, name);

    if (patch != NULL)
        return patch;

    patch = add_native_patch(world);
    set_string(&patch->name, name);
    return patch;
}

void remove_native_patch(World* world, const char* name)
{
    // TODO
}

CIRCA_EXPORT void circa_patch_function(caNativePatch* patch, const char* nameStr,
        caEvaluateFunc func)
{
    Value name;
    set_string(&name, nameStr);

    Value* funcIndex = hashtable_get(&patch->funcMap, &name);

    if (funcIndex == NULL) {
        int index = add_native_func(patch->world, &name, func);
        set_int(hashtable_insert(&patch->funcMap, &name), index);
    } else {
        NativeFunc* entry = &patch->world->funcTable[as_int(funcIndex)];
        copy(&name, &entry->name);
        entry->func = func;
    }
}

#if 0
void native_patch_apply_patch(NativePatch* module, Block* block)
{
    // Walk through list of patches, and apply them to block terms as appropriate.
    
    for (HashtableIterator it(&module->patches); it; ++it) {
        caValue* patchType = it.key()->element(0);
        caValue* name = it.key()->element(1);

        Term* term = find_local_name(block, name);

        if (term == NULL)
            continue;

        switch (as_symbol(patchType)) {
        case sym_Function: {
            if (!is_function(term))
                break;

            EvaluateFunc func = (EvaluateFunc) as_opaque_pointer(it.value());
            #if NATIVE_PATCH_VERBOSE
                printf("Patching with native func: %s\n", as_cstring(name));
            #endif
            install_function(term, func);
            break;
        }

        case sym_TypeRelease: {
            if (!is_type(term))
                break;

            ReleaseFunc func = (ReleaseFunc) as_opaque_pointer(it.value());
            as_type(term_value(term))->release = func;
            #if NATIVE_PATCH_VERBOSE
                printf("Patching with native release func: %s\n", as_cstring(name));
            #endif
            break;
        }
        }
    }
}
#endif

void native_patch_finish_change(NativePatch* module)
{
#if 0
    World* world = module->world;

    // Apply changes to the target module.
    caValue* targetName = &module->targetName;
    Block* block = nested_contents(find_from_global_name(world, as_cstring(targetName)));
    if (block == NULL)
        // It's okay if the block doesn't exist yet.
        return;

    native_patch_apply_patch(module, block);
#endif
}

CIRCA_EXPORT void circa_finish_native_patch(caNativePatch* module)
{
    native_patch_finish_change(module);
}

#if 0
static NativePatch* find_native_patch_for_module(World* world, Block* module)
{
    if (module == NULL)
        return NULL;

    StringToNativePatchMap::const_iterator it = world->nativeModules.find(
        as_cstring(term_name(module->owningTerm)));

    if (it == world->nativeModules.end())
        return NULL;

    return it->second;
}
#endif

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
#if 0
    Term* term = function->owningTerm;
    if (!term_can_be_patched(term))
        return;

    World* moduleWorld = world->nativePatchWorld;

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
#endif
    #if NATIVE_PATCH_VERBOSE
        printf("Patching with native func (on creation): %s\n", as_cstring(term_name(term)));
    #endif
}

NativeFuncIndex find_native_func_index(World* world, Block* block)
{
    if (block->owningTerm == NULL)
        return -1;

    Value* funcName = term_name(block->owningTerm);
    if (is_null(funcName))
        return -1;

    Block* module = find_enclosing_module(block);
    if (module == NULL)
        return -1;

    NativePatch* nativePatch = find_existing_native_patch(world,
        as_cstring(term_name(module->owningTerm)));

    if (nativePatch == NULL)
        return -1;

    Value* patchIndex = hashtable_get(&nativePatch->funcMap, funcName);
    if (patchIndex == NULL)
        return -1;

    return patchIndex->asInt();
}

NativeFuncIndex find_native_func_index_by_name(NativePatch* patch, Value* name)
{
    Value* patchIndex = hashtable_get(&patch->funcMap, name);
    if (patchIndex == NULL)
        return -1;
    return as_int(patchIndex);
}

EvaluateFunc get_native_func(World* world, NativeFuncIndex index)
{
    ca_assert(index != -1 && index < world->funcTableCount);
    return world->funcTable[index].func;
}

Value* get_native_func_name(World* world, NativeFuncIndex index)
{
    if (index < 0 || index >= world->funcTableCount)
        return NULL;

    return &world->funcTable[index].name;
}

void native_patch_apply_to_new_type(World* world, Type* type)
{
#if 0
    Term* term = type->declaringTerm;
    if (!term_can_be_patched(term))
        return;

    World* moduleWorld = world->nativePatchWorld;

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
#endif
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


#if 0
CIRCA_EXPORT void circa_patch_type_release(caNativePatch* module, const char* typeName, caReleaseFunc func)
{
    module_patch_type_release((NativePatch*) module, typeName, func);
}
#endif

} // namespace circa
