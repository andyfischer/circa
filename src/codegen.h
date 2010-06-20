// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

// codegen.cpp
//
// Classes that take compiled Circa code and generate equivalent code in another
// language.
//

#pragma once

#include "common_headers.h"

namespace circa {

std::string generate_cpp_headers(Branch& branch);

} // namespace circa
