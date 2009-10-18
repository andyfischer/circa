// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#ifndef PLASTIC_GL_COMMON_INCLUDED
#define PLASTIC_GL_COMMON_INCLUDED

#include "common_headers.h"

void gl_clear_error();

// If OpenGL reports an error, we'll attach the error to the given term and return true.
bool gl_check_error(circa::Term* errorListener, const char* note="");

#endif
