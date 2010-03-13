// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_GENERATE_DOCS_INCLUDED
#define CIRCA_GENERATE_DOCS_INCLUDED

#include "common_headers.h"

namespace circa {

std::string escape_newlines(std::string s);
void hide_from_docs(Term* term);
void append_package_docs(std::stringstream& out, Branch& branch, std::string const& package_name);
void initialize_kernel_documentation(Branch& KERNEL);

} // namespace circa

#endif
