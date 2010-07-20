// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#ifdef WINDOWS

// Prevent warnings on getenv()
#define _CRT_SECURE_NO_WARNINGS 1

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glew.h>

#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

namespace circa {

struct Branch;
struct EvalContext;
struct FeedbackOperation;
struct Function;
class ReferenceIterator;
struct Term;
struct Type;
struct Ref;
struct RefList;
struct ReferenceMap;

} // namespace circa

#ifdef PLASTIC_OSX
    #define PLASTIC_USE_SDL
#endif


#ifdef PLASTIC_USE_SDL
    #define NO_SDL_GLEXT

    #include <SDL.h>
    #include <SDL_opengl.h>
    #include <SDL_image.h>

#endif // PLASTIC_USE_SDL

#ifdef PLASTIC_OSX

    #include <OpenGL/glu.h>
    #include <OpenGL/glext.h>

#endif

#ifdef PLASTIC_IPAD

    #define PLASTIC_OGL_ES
    #define USE_OPENFRAMEWORKS

    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>

    // TODO: Add a flag that disables ES2 compilation so that it's easy to check that the
    // app is ES1 compatible.
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

#endif
