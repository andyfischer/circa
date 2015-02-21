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
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "modules.h"
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
    NativePatch* patch = new NativePatch();
    
    set_hashtable(&patch->funcMap);
    patch->world = world;
    patch->dll = NULL;

    world->nativePatchCount++;
    world->nativePatch = (NativePatch**) realloc(world->nativePatch,
        sizeof(NativePatch*) * world->nativePatchCount);
    world->nativePatch[world->nativePatchCount-1] = patch;

    return patch;
}

void free_native_patch(NativePatch* patch)
{
    delete patch;
}

NativePatch* find_existing_native_patch(World* world, Value* name)
{
    for (int i=0; i < world->nativePatchCount; i++) {
        NativePatch* patch = world->nativePatch[i];
        if (equals(&patch->name, name))
            return patch;
    }

    return NULL;
}

NativePatch* insert_native_patch(World* world, Value* name)
{
    NativePatch* patch = find_existing_native_patch(world, name);

    if (patch != NULL)
        return patch;

    patch = add_native_patch(world);
    set_value(&patch->name, name);

    // Make sure this module is loaded by a global name. The native patch won't
    // work if the module has only been loaded by filename or relative name.
    load_module(world, NULL, name);

    return patch;
}

void remove_native_patch(World* world, const char* name)
{
    // TODO
}

CIRCA_EXPORT void circa_patch_function2(caNativePatch* patch, const char* nameStr,
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

CIRCA_EXPORT void circa_finish_native_patch(caNativePatch* module)
{
    // This once did something.
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

    Value* moduleName = block_get_property(module, s_Name);
    if (moduleName == NULL)
        return -1;

    NativePatch* nativePatch = find_existing_native_patch(world, moduleName);

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

void native_patch_add_platform_specific_suffix(Value* filename)
{
    // Future: Windows support.
    string_append(filename, ".so");
}

void native_patch_close(NativePatch* module)
{
#ifdef CIRCA_DISABLE_DLL
    internal_error("native_patch_close failed: this binary was not compiled with DLL support");
#else
    if (module->dll != NULL)
        dlclose(module->dll);

    module->dll = NULL;
#endif
}

void native_patch_load_from_file(NativePatch* module, const char* filename)
{
#ifdef CIRCA_DISABLE_DLL
    internal_error("native_patch_load_from_file failed: this binary was not compiled with DLL support");
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
    Value nameStr;
    set_string(&nameStr, name);
    return insert_native_patch(world, &nameStr);
}

} // namespace circa
