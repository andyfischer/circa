// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

#define resolve_possible_module_path circa_resolve_possible_module_path

// -- Search paths --
Value* module_search_paths(World* world);
void module_add_search_path(World* world, const char* str);

// -- Filenames --
Block* create_module(World* world);
void module_set_name(World* world, Block* block, Value* name);
void module_set_filename(World* world, Block* block, Value* filename);

void module_install_replacement(World* world, Value* filename, Block* block);

Block* find_module(World* world, Value* relativeDir, Value* name);
Block* find_module_by_filename(World* world, Value* filename);
void find_module_file_local(World* world, Block* loadedBy, Value* moduleName, Value* filenameOut);

Block* load_module(World* world, Value* relativeDir, Value* moduleName);
Block* load_module_relative_to(World* world, Block* relativeTo, Value* moduleName);
Block* load_module_by_filename(World* world, Value* filename);

Block* module_ref_resolve(World* world, Value* module);

Term* module_lookup(Block* module, Term* caller);
Term* module_ref_lookup(Value* moduleRef, Term* caller);
Term* module_lookup(World* world, Value* moduleRef, Value* name);

Value* find_enclosing_filename(Block* block);
void find_enclosing_dirname(Block* block, Value* result);

bool is_module_ref(Value* value);
void set_module_ref(Value* value, Value* name, Value* relativeDir);

} // namespace circa
