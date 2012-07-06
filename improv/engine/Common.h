// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include <OpenGL/gl.h>
#include <glm/glm.hpp>
#include "circa/circa.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
    
    Color() : r(1.0f),g(1.0f),b(1.0f),a(1.0f) {}
};

void Log(const char* fmt, ...);
int NextPowerOfTwo(int i);
