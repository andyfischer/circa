// Copyright 2008 Paul Hodge

#ifndef CIRCA_COMPILATION_INCLUDED
#define CIRCA_COMPILATION_INCLUDED

#include "common_headers.h"

namespace circa {

bool has_compile_errors(Branch& branch);
std::vector<std::string> get_compile_errors(Branch& branch);
void print_compile_errors(Branch& branch, std::ostream& output);

} // namespace circa

#endif
