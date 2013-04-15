// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

caValue* module_search_paths(World* world);

void module_add_search_path(World* world, const char* str);

void module_get_default_name_from_filename(caValue* filename, caValue* moduleNameOut);

// Add a top-level with the given name, or return an existing one if it exists.
Block* fetch_module(World* world, const char* name);

// Load a module from the given filename. If the module already exists, then we'll replace
// the existing contents, and we'll update any existing references that point to the replaced
// code. Does not create a file watch (see load_module_file_watched).
Block* load_module_file(World* world, const char* moduleName, const char* filename);

// Loads a module from the given filename, and creates a file watch.
Block* load_module_file_watched(World* world, const char* moduleName, const char* filename);

// Load a module via name. The file will be found via standard module lookup.
Block* load_module_by_name(World* world, const char* module_name);

// This should be called whenever a new module is loaded by a certain term. We may
// rearrange the global module order so that the module is located before the term.
void module_on_loaded_by_term(Block* module, Term* loadCall);

Block* find_loaded_module(const char* name);

Block* find_module_from_filename(const char* filename);

// Install builtin modules functions.
void modules_install_functions(Block* kernel);

} // namespace circa
