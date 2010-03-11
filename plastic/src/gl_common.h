// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef PLASTIC_GL_COMMON_INCLUDED
#define PLASTIC_GL_COMMON_INCLUDED

#include "common_headers.h"

void gl_clear_error();

bool gl_check_error(char* buf);

// If OpenGL reports an error, we'll call error_occurred and return true.
bool gl_check_error(circa::EvalContext* cxt, circa::Term* term);

#endif
