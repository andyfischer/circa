// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef CIRCA_GENERATE_DOCS_INCLUDED
#define CIRCA_GENERATE_DOCS_INCLUDED

#include "common_headers.h"

namespace circa {

void append_package_docs(std::stringstream& out, Branch& branch, std::string const& package_name);
//void generate_docs(Branch& branch, std::stringstream &out);
void initialize_kernel_documentation(Branch& KERNEL);

} // namespace circa

#endif
