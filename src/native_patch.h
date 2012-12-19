// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef void (*OnModuleLoad)(NativePatch* nativeModule);

struct NativePatchWorld;
struct NativePatch;

NativePatchWorld* create_native_patch_world();

NativePatch* get_existing_native_module(World* world, const char* name);

// Add a module with the given unique name to the World. If a module with this name already
// exists, return the existing one. The name is usually the filename.
NativePatch* add_native_patch(World* world, const char* name);
void remove_native_patch(World* world, const char* name);

// Add a function patch on the given module.
void module_patch_function(NativePatch* module, const char* name, EvaluateFunc func);

// Manually apply a module's patches to the given block. This function isn't commonly used.
// (the common way is to allow patches on the World to be automatically applied).
void native_module_apply_patch(NativePatch* module, Block* block);

void native_module_finish_change(NativePatch* module);

void module_possibly_patch_new_function(World* world, Block* function);

void native_module_add_platform_specific_suffix(caValue* filename);
void native_module_load_from_file(NativePatch* module, const char* filename);

} // namespace circa
