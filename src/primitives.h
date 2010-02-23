// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_PRIMITIVES_INCLUDED
#define CIRCA_PRIMITIVES_INCLUDED

#include "common_headers.h"

namespace circa {

extern Term* ANY_TYPE;
extern Term* BOOL_TYPE;
extern Term* FLOAT_TYPE;
extern Term* INT_TYPE;
extern Term* REF_TYPE;
extern Term* STRING_TYPE;

void initialize_primitive_types(Branch& kernel);

// Do some more setup, after all the standard builtin types have been created.
void post_setup_primitive_types();

} // namespace circa

#endif // CIRCA_PRIMITIVES_INCLUDED
