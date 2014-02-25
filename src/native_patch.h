// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef void (*OnModuleLoad)(NativePatch* nativeModule);

struct NativePatchWorld;
struct NativePatch;

NativePatchWorld* alloc_native_patch_world();
void free_native_patch_world(NativePatchWorld* world);

NativePatch* get_existing_native_patch(World* world, const char* name);

NativePatch* alloc_native_patch(World* world);
void free_native_patch(NativePatch* patch);

// Add a module with the given unique name to the World. If a module with this name already
// exists, return the existing one. The name is usually the filename.
NativePatch* insert_native_patch(World* world, const char* name);
void remove_native_patch(World* world, const char* name);

// Add a function patch on the given module.
void module_patch_function(NativePatch* module, const char* name, EvaluateFunc func);
void module_patch_type_release(NativePatch* module, const char* typeName, ReleaseFunc func);

// Manually apply a module's patches to the given block. This function isn't commonly used.
// (the common way is to allow patches on the World to be automatically applied).
void native_patch_apply_patch(NativePatch* module, Block* block);

void native_patch_finish_change(NativePatch* module);

void native_patch_apply_to_new_function(World* world, Block* function);
void native_patch_apply_to_new_type(World* world, Type* type);

void native_patch_add_platform_specific_suffix(caValue* filename);
void native_patch_load_from_file(NativePatch* module, const char* filename);

} // namespace circa
