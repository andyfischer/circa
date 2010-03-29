// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_COMMON_HEADERS_INCLUDED
#define CIRCA_COMMON_HEADERS_INCLUDED

#ifdef WINDOWS

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h> 

#endif // WINDOWS

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeinfo>

namespace circa {

struct Branch;
struct EvalContext;
struct FeedbackOperation;
struct Function;
struct FunctionAttrs;
struct TaggedValue;
struct Term;
struct Type;
struct Ref;
struct RefList;
struct ReferenceMap;
struct StyledSource;

} // namespace circa

#endif
