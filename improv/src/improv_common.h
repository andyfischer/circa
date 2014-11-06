// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#ifdef NACL
  #ifdef IMPROV_REGAL
    #include <GL/Regal.h>
  #else
    #include "ppapi/lib/gl/gles2/gl2ext_ppapi.h"
    #include <GLES2/gl2.h>
  #endif
#else
  #include <OpenGL/gl.h>
#endif

#include <cstddef>
#include <cstring>

#include "circa/circa.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace improv {

void Log(const char* msg);
void Log(const char* msg, caValue* arg1);
void Log(const char* msg, const char* arg1);
int NextPowerOfTwo(int i);

}

#ifdef NACL

  #define IMPROV_USE_SDL 0

#else

  #define IMPROV_USE_SDL 1

#endif

#define IMPROV_USE_CAIRO 1
