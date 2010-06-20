// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

int int_input(Term* term, int index);
float float_input(Term* term, int index);
bool bool_input(Term* term, int index);
const char* string_input(Term* term, int index);

} // namespace circa
