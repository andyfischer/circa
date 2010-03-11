// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "plastic.h"

void gl_clear_error()
{
    glGetError();
}

bool gl_check_error(char* buf)
{
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
    sprintf(buf, "OpenGL reported error: %s", enums[errcode]);
    return true;
}

bool gl_check_error(circa::EvalContext* cxt, circa::Term* term)
{
    if (cxt->errorOccurred)
        return true;

    char buf[50];
    bool error = gl_check_error(buf);

    if (error) {
        circa::error_occurred(cxt, term, buf);
    }

    return error;
}
