// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#ifndef PLASTIC_COMMON_HEADERS_INCLUDED
#define PLASTIC_COMMON_HEADERS_INCLUDED

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
struct FeedbackOperation;
struct Function;
class ReferenceIterator;
struct Term;
struct Type;
struct Ref;
struct RefList;
struct ReferenceMap;

} // namespace circa

#define NO_SDL_GLEXT

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef __APPLE__

#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#endif

#endif
