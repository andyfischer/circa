// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

Term* find_named(Branch const& branch, std::string const& name);

Term* get_named(Branch const& branch, std::string const& qualifiedName);

// char* overload for use in GDB
Term* get_named(Branch const& branch, const char* name);

// If the string is a qualified name (such as "a:b:c"), returns the index
// of the first colon. If the string isn't a qualified name then returns
// strlen(name).
unsigned find_qualified_name_separator(const char* name);

// If the string is a qualified name (such as "a:b:c"), returns the string
// of everything after the first colon (such as "b:c"). If it isn't a
// qualified name, returns NULL.
const char* split_qualified_name(const char* name);

// Find the name binding that is active at the given branch index.
Term* get_named_at(Branch& branch, int index, std::string const& name);
Term* get_named_at(Term* location, std::string const& name);

// Get a named term from the global namespace.
Term* get_global(std::string name);

Branch* get_parent_branch(Branch& branch);
Term* get_parent_term(Term* term);
bool name_is_reachable_from(Term* term, Branch& branch);
Branch* find_first_common_branch(Term* left, Term* right);

// Get a name of 'term' which is valid in 'branch'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Branch& branch, Term* term);
std::string get_relative_name(Term* location, Term* term);

void update_unique_name(Term* term);
const char* get_unique_name(Term* term);

// Take all named terms in 'source' and bind them to the same name in 'destination'.
// This is used for joining branches in if/for blocks.
void expose_all_names(Branch& source, Branch& destination);

Term* find_from_unique_name(Branch& branch, const char* name);

} // namespace circa
