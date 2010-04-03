// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "plastic_common_headers.h"

void gl_clear_error();

// If OpenGL reports an error, we'll call error_occurred and return true.
bool gl_check_error(circa::EvalContext* cxt, circa::Term* term);

const char* gl_check_error();
const char* gl_to_string(GLenum glenum);
