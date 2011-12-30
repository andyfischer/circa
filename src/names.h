// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

Term* find_name(Branch* branch, const char* name);
Term* find_name_at(Term* location, const char* name);

Term* find_local_name(Branch* branch, int location, const char* name);
Term* find_local_name(Branch* branch, const char* name);

// If the string is a qualified name (such as "a:b:c"), returns the index
// of the first colon. If the string isn't a qualified name then returns
// -1.
int find_qualified_name_separator(const char* name);

// Get a named term from the global namespace.
Term* get_global(std::string name);

Branch* get_parent_branch(Branch* branch);
Term* get_parent_term(Term* term);
Term* get_parent_term(Term* term, int levels);
bool name_is_reachable_from(Term* term, Branch* branch);
Branch* find_first_common_branch(Term* left, Term* right);

// Get a name of 'term' which is valid in 'branch'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Branch* branch, Term* term);
std::string get_relative_name_at(Term* location, Term* term);

void update_unique_name(Term* term);
const char* get_unique_name(Term* term);

Term* find_from_unique_name(Branch* branch, const char* name);

// Attempts to find a global name for the term. If successful, returns true
// and writes the result to 'name'. If unsuccessful (for example, if the
// Branch is locally-allocated), returns false.
bool find_global_name(Term* term, std::string& name);

// Convenience function, calls find_global_name and returns a blank string if
// it wasn't found.
std::string find_global_name(Term* term);

Term* find_term_from_global_name(const char* name);

} // namespace circa
