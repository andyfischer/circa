// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void initialize_compound_type(Type* type);

Term* create_compound_type(Branch& branch, std::string const& name);

Type* get_compound_list_element_type(Type* compoundType, int index);

}
