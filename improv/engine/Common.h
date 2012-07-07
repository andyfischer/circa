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
    Color(GLfloat _r, GLfloat _g, GLfloat _b) : r(_r),g(_g),b(_b),a(1.0f) {}
    Color(GLfloat _r, GLfloat _g, GLfloat _b, GLfloat _a) : r(_r),g(_g),b(_b),a(_a) {}
};

void Log(const char* msg);
void Log(const char* msg, caValue* arg1);
void Log(const char* msg, const char* arg1);
int NextPowerOfTwo(int i);
