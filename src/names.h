// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

namespace circa {

Term* find_named(Branch const& branch, std::string const& name);

Term* get_named(Branch const& branch, std::string const& qualifiedName);

bool name_is_reachable_from(Term* term, Branch& branch);

// Get a name of 'term' which is valid in 'branch'. This might simply return term's name,
// or if term is inside a namespace or object, this would return a colon-separated name.
std::string get_relative_name(Branch& branch, Term* term);
std::string get_relative_name(Term* location, Term* term);

// Take all named terms in 'source' and bind them to the same name in 'destination'.
// This is used for joining branches in if/for blocks.
void expose_all_names(Branch& source, Branch& destination);

} // namespace circa
