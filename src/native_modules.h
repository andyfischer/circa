// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef void (*OnModuleLoad)(NativeModule* nativeModule);

struct NativeModuleWorld;
struct NativeModule;

NativeModuleWorld* create_native_module_world();

// Create a new NativeModule object. This function isn't commonly used (you might want
// add_native_module instead).
NativeModule* create_native_module(World* world);
void free_native_module(NativeModule* module);

// Add a module with the given unique name to the World. If a module with this name already
// exists, return the existing one. The name is usually the filename.
NativeModule* add_native_module(World* world, const char* name);
void delete_native_module(World* world, const char* name);

// Add a function patch on the given module.
void module_patch_function(NativeModule* module, const char* name, EvaluateFunc func);

// Manually apply a module's patches to the given branch. This function isn't commonly used.
// (the common way is to allow patches on the World to be automatically applied).
void native_module_apply_patch(NativeModule* module, Branch* branch);

void native_module_add_change_action_patch_branch(NativeModule* module, const char* branchName);

void native_module_on_change(NativeModule* module);

void module_on_loaded_branch(Branch* branch);
void module_possibly_patch_new_function(World* world, Branch* function);

void native_module_add_platform_specific_suffix(caValue* filename);
void native_module_load_from_file(NativeModule* module, const char* filename);

} // namespace circa
