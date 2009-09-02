// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

namespace circa {

Term* find_named(Branch& branch, std::string const& name);

Term* get_named(Branch& branch, std::string const& name);

// Look for a dot separated name. Ie, with an input of a.b.c, we'll look for 'a',
// then if found we'll look inside that for 'b', etc.
Term* get_dot_separated_name(Branch& branch, std::string const& name);

void expose_all_names(Branch& source, Branch& destination);

} // namespace circa
