// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

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

NativeModule* add_native_module(World* world, const char* name)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    // Return existing module, if it exists.
    std::map<std::string, NativeModule*>::const_iterator it = moduleWorld->nativeModules.find(name);
    if (it != moduleWorld->nativeModules.end())
        return it->second;

    // Create module.
    NativeModule* module = create_native_module();
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

void module_manually_patch_branch(NativeModule* module, Branch* branch)
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

void module_apply_patches_to_function(World* world, Branch* functionBranch)
{
    Term* term = functionBranch->owningTerm;
    Function* function = as_function(term);

    // TODO
}

void module_on_loaded_branch(Branch* branch)
{
    // Search the script for calls to native_patch.
    for (int i=0; i < branch->length(); i++) {
        Term* term = branch->get(i);
        if (term->function != FUNCS.native_patch)
            continue;

        // Load the native module.
        Value filename;
        get_path_relative_to_source(branch, term_value(term->input(0)), &filename);

        Value fileWatchAction;

    }
    
    // Apply any existing patches.
    // TODO
    World* world = global_world();
}

void native_module_add_platform_specific_suffix(caValue* filename)
{
    string_append(filename, ".so");
}

void native_module_load_from_file(NativeModule* module, const char* filename)
{
    if (module->dll != NULL)
        dlclose(module->dll);

    module->dll = dlopen(filename, RTLD_NOW);

    if (module->dll) {
        std::cout << "failed to open dll: " << filename << std::endl;
        return;
    }

    OnModuleLoad onModuleLoad = dlsym(module->dll, "circa_module_load");

    if (onModuleLoad == NULL) {
        std::cout << "could not find circa_module_load in: " << filename << std::endl;
        return;
    }

    onModuleLoad(module);
}

EXPORT void circa_module_patch_function(caNativeModule* module, const char* name, caEvaluateFunc func)
{
    module_patch_function(module, name, func);
}

} // namespace circa
