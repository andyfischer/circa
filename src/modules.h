// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

caValue* module_search_paths(World* world);

void module_add_search_path(World* world, const char* str);

void module_get_default_name_from_filename(caValue* filename, caValue* moduleNameOut);

Branch* load_module_from_file(const char* module_name, const char* filename);

// Add a top-level with the given name, or return an existing one if it exists.
Branch* add_module(World* world, const char* name);

// Load a module via name. The file will be found via standard module lookup.
Branch* load_module_by_name(World* world, const char* module_name);

// This should be called whenever a new module is loaded by a certain term. We may
// rearrange the global module order so that the module is located before the term.
void module_on_loaded_by_term(Branch* module, Term* loadCall);

Branch* find_loaded_module(const char* name);

Branch* find_module_from_filename(const char* filename);

// Updating & migration
Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch);

// Look through every term in 'target', and see if it contains a reference to a term in
// 'oldBranch'. If so, migrate that reference to the equivalent term (according to the
// relative unique names) inside 'newBranch'. If the equivalent in 'newBranch' isn't found,
// then the reference will be set to null.
void update_all_code_references(Branch* target, Branch* oldBranch, Branch* newBranch);

// Install builtin modules functions.
void modules_install_functions(Branch* kernel);

} // namespace circa
