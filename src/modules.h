// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

List* modules_get_search_paths();

void modules_add_search_path(const char* str);

Branch* load_module_from_file(const char* module_name, const char* filename);

Branch* load_module(const char* module_name, Term* loadCall);

Branch* find_loaded_module(const char* name);

Branch* find_module_from_filename(const char* filename);

// Updating & migration
Term* translate_term_across_branches(Term* term, Branch* oldBranch, Branch* newBranch);

// Look through every term in 'target', and see if it contains a reference to a term in
// 'oldBranch'. If so, migrate that reference to the equivalent term (according to the
// relative unique names) inside 'newBranch'. If the equivalent in 'newBranch' isn't found,
// then the reference will be set to null.
void update_all_code_references(Branch* target, Branch* oldBranch, Branch* newBranch);

void get_relative_name(Term* term, Branch* relativeTo, caValue* nameOutput);
Term* find_from_relative_name(caValue* name, Branch* relativeTo);

} // namespace circa
