// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

typedef void (*OnModuleLoad)(NativePatch* nativeModule);
typedef int NativeFuncIndex;

struct NativeFunc;
struct NativePatch;

NativePatch* get_existing_native_patch(World* world, const char* name);

// Add a module with the given unique name to the World. If a module with this name already
// exists, return the existing one. The name is usually the filename.
NativePatch* insert_native_patch(World* world, Value* name);
void remove_native_patch(World* world, const char* name);

// Add a function patch on the given module.
void module_patch_function(NativePatch* module, const char* name, EvaluateFunc func);

void native_patch_finish_change(NativePatch* module);

void native_patch_add_platform_specific_suffix(caValue* filename);
void native_patch_load_from_file(NativePatch* module, const char* filename);

NativeFuncIndex find_native_func_index(World* world, Block* block);
NativeFuncIndex find_native_func_index_by_name(NativePatch* patch, Value* name);
EvaluateFunc get_native_func(World* world, NativeFuncIndex index);
Value* get_native_func_name(World* world, NativeFuncIndex index);

} // namespace circa
