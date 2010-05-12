// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

void initialize_compound_type(Type* type);
void initialize_compound_type(Term* term);

Term* create_compound_type(Branch& branch, std::string const& name);

}
