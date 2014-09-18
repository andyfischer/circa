// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// -- Search paths --
Value* module_search_paths(World* world);
void module_add_search_path(World* world, const char* str);

// -- Filenames --
Block* create_module(World* world);
void module_set_name(World* world, Block* block, Value* name);
void module_set_filename(World* world, Block* block, Value* filename);

void module_install_replacement(World* world, Value* filename, Block* block);

Block* find_module(World* world, Value* name);
Block* find_module_by_filename(World* world, Value* filename);

Block* load_module(World* world, Block* relativeTo, Value* moduleName);
Block* load_module_by_filename(World* world, Value* filename);

void resolve_possible_module_path(World* world, Value* path, Value* result);

Block* module_ref_get_block(Value* module);

Term* module_lookup(Value* module, Term* caller);

bool is_module_ref(Value* value);
void set_module_ref(Value* value, Block* block);

// Install builtin modules functions.
void modules_install_functions(NativePatch* patch);

} // namespace circa
