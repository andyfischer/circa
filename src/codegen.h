// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// codegen.cpp
//
// Classes that take compiled Circa code and generate equivalent code in another
// language.
//

#ifndef CIRCA_CODEGEN_INCLUDED
#define CIRCA_CODEGEN_INCLUDED

#include "common_headers.h"

namespace circa {

std::string generate_cpp_headers(Branch& branch);

} // namespace circa

#endif // CIRCA_CODEGEN_INCLUDED
