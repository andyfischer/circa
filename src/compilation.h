// Copyright 2008 Andrew Fischer

#ifndef CIRCA__COMPILATION__INCLUDED
#define CIRCA__COMPILATION__INCLUDED

#include "common_headers.h"

namespace circa {

bool hasCompileErrors(Branch* branch);
std::vector<std::string> getCompileErrors(Branch* branch);
void printCompileErrors(Branch* branch, std::ostream& output);

} // namespace circa

#endif
