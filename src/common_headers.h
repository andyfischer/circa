// Copyright 2008 Andrew Fischer

#ifndef CIRCA_COMMON_HEADERS_INCLUDED
#define CIRCA_COMMON_HEADERS_INCLUDED

#ifdef WINDOWS

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>

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
class ReferenceIterator;
struct Term;
struct Type;
struct Ref;
struct RefList;
struct ReferenceMap;

} // namespace circa

#endif
