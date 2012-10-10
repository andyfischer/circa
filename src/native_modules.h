// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct NativeModuleWorld;
struct NativeModule;
struct NativePatchFunction;

NativeModuleWorld* create_native_module_world();

NativeModule* create_native_module();
void free_native_module(NativeModule* module);

void module_patch_function(NativeModule* module, Name name, EvaluateFunc func);
void module_patch_function(NativeModule* module, const char* name, EvaluateFunc func);

void module_manually_patch_branch(NativeModule* module, Branch* branch);

void module_on_loaded_branch(Branch* branch);

} // namespace circa
