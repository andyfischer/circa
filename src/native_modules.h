// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NativeModuleWorld;
struct NativeModule;
struct NativePatchFunction;

NativeModuleWorld* create_native_module_world();

// Create a new NativeModule object. This function isn't commonly used (you might want
// add_native_module instead).
NativeModule* create_native_module();
void free_native_module(NativeModule* module);

// Add a module with the given name to the World. If a module with this name already exists,
// return the existing one.
NativeModule* add_native_module(World* world, Name name);
void delete_native_module(World* world, Name name);

// Add a patch on the given module.
void module_patch_function(NativeModule* module, Name name, EvaluateFunc func);
void module_patch_function(NativeModule* module, const char* name, EvaluateFunc func);

// Manually apply a module's patches to the given branch. This function isn't commonly used
// (the 'common' way is to allow patches on the World to be automatically applied).
void module_manually_patch_branch(NativeModule* module, Branch* branch);

// Possibly apply a patch to a new function.
void module_apply_patches_to_function(World* world, Branch* function);

void module_on_loaded_branch(Branch* branch);

} // namespace circa
