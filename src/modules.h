// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// -- Search paths --
caValue* module_search_paths(World* world);
void module_add_search_path(World* world, const char* str);

// -- Filenames --
void module_get_default_name_from_filename(caValue* filename, caValue* moduleNameOut);

// Add a top-level with the given name, or return an existing one if it exists.
Block* fetch_module(World* world, const char* name);

// Load a module from the given filename. If the module already exists, then we'll replace
// the existing contents, and we'll update any existing references that point to the replaced
// code. Does not create a file watch (see load_module_file_watched).
Block* load_module_file(World* world, caValue* moduleName, const char* filename);

void install_block_as_module(World* world, caValue* moduleName, Block* block);

// Loads a module from the given filename, and creates a file watch.
Block* load_module_file_watched(World* world, caValue* moduleName, const char* filename);

// Load a module via name. The file will be found via standard module lookup.
Block* load_module_by_name(World* world, Block* loadedBy, caValue* moduleName);

// This should be called whenever a new module is loaded by a certain term. We may
// rearrange the global module order so that the module is located before the term.
void module_on_loaded_by_term(Block* module, Term* loadCall);

// Deprecated: does not reference World or Block.
Block* find_loaded_module(const char* name);
Block* find_module_from_filename(const char* filename);

// Prefered:
Block* find_module(Block* root, caValue* name);

// Runtime calls (when evaluating require)
#if 0
bool module_is_loaded_in_stack(Stack* stack, caValue* moduleRef);
caValue* module_insert_in_stack(Stack* stack, caValue* moduleRef);
caValue* module_get_stack_contents(Stack* stack, caValue* moduleRef);
void module_capture_exports_from_stack(Frame* frame, caValue* output);
caValue* module_find_closure_on_stack(Stack* stack, Term* function);
#endif
Block* module_ref_get_block(caValue* moduleRef);

// -- Bundles --
Block* module_create_empty_bundle(World* world, const char* name);

#if CIRCA_ZIP_SUPPORT
Block* module_load_from_zip_data(char* data, size_t data);
#endif

bool is_module_ref(caValue* value);

// Install builtin modules functions.
void modules_install_functions(Block* kernel);

} // namespace circa
