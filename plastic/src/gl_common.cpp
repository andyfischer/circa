// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "plastic.h"

void gl_clear_error()
{
    glGetError();
}

bool gl_check_error(circa::Term* errorListener, const char* note)
{
    if (errorListener->hasError())
        return true;

    // With thanks to Philip Rideout for example code. See: http://prideout.net/bloom/index.php
    char enums[][20] =
    {
        "invalid enumeration", // GL_INVALID_ENUM
        "invalid value",       // GL_INVALID_VALUE
        "invalid operation",   // GL_INVALID_OPERATION
        "stack overflow",      // GL_STACK_OVERFLOW
        "stack underflow",     // GL_STACK_UNDERFLOW
        "out of memory"        // GL_OUT_OF_MEMORY
    };

    GLenum errcode = glGetError();
    if (errcode == GL_NO_ERROR)
        return false;

    errcode -= GL_INVALID_ENUM;

    char buf[50];
    sprintf(buf, "OpenGL reported error: %s%s", enums[errcode], note);

    circa::error_occurred(errorListener, buf);

    return true;
}
