// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <vector>
#include <map>

#include "branch.h"
#include "function.h"
#include "kernel.h"
#include "names.h"
#include "native_modules.h"
#include "term.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

struct NativeModuleWorld
{
    std::map<Name, NativeModule*> nativeModules;
};

struct NativeModule
{
    std::map<Name, EvaluateFunc> patches;
    Name namePrefix;

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
    module->namePrefix = name_None;
    return module;
}

void free_native_module(NativeModule* module)
{
    delete module;
}

NativeModule* add_native_module(World* world, Name name)
{
    NativeModuleWorld* moduleWorld = world->nativeModuleWorld;

    // Return existing module, if it exists.
    std::map<Name, NativeModule*>::const_iterator it = moduleWorld->nativeModules.find(name);
    if (it != moduleWorld->nativeModules.end())
        return it->second;

    // Create module.
    NativeModule* module = create_native_module();
    moduleWorld->nativeModules[name] = module;
    return module;
}

void delete_native_module(World* world, Name name)
{
    world->nativeModuleWorld->nativeModules.erase(name);
}

EvaluateFunc module_find_patch_for_name(NativeModule* module, Name name)
{
    std::map<Name, EvaluateFunc>::const_iterator it = module->patches.find(name);
    if (it == module->patches.end())
        return NULL;
    return it->second;
}

void module_set_name_prefix(NativeModule* module, Name name)
{
    module->namePrefix = name;
}

void module_patch_function(NativeModule* module, Name name, EvaluateFunc func)
{
    module->patches[name] = func;
}

void module_patch_function(NativeModule* module, const char* nameStr, EvaluateFunc func)
{
    Name name = name_from_string(nameStr);
    module_patch_function(module, name, func);
}

void module_manually_patch_branch(NativeModule* module, Branch* branch)
{
    bool anyTouched = false;

    // If the module has a name prefix, then dig down to that branch.
    if (module->namePrefix != name_None) {
        Term* prefixTerm = find_name(branch, module->namePrefix);

        if (prefixTerm == NULL) {
            // Prefix name was not found, so nothing to patch. This should probably
            // be reported as an error.
            return;
        }

        branch = nested_contents(prefixTerm);
    }

    // Walk through list of patches, and try to find any functions to apply them to.
    std::map<Name, EvaluateFunc>::const_iterator it;
    for (it = module->patches.begin(); it != module->patches.end(); ++it) {
        Name name = it->first;
        EvaluateFunc evaluateFunc = it->second;

        Term* term = find_name(branch, name);

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
    // Search the script for calls to load_native_patch.
    
    // Apply any existing patches.
    World* world = global_world();
}

} // namespace circa

// Public functions
void circa_module_patch_function(caNativeModule* module, const char* name, caEvaluateFunc func)
{
    circa::module_patch_function(module, name, func);
}
