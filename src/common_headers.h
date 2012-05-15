// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#ifdef WINDOWS

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h> 

#endif // WINDOWS

// standard libraries
#include <cmath>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <stdlib.h>

#define EXPORT extern "C"

#include "circa/circa.h"

namespace circa {

struct Branch;
struct BrokenLinkList;
struct CastResult;
struct CircaObject;
struct Dict;
struct Frame;
struct Stack;
struct FeedbackOperation;
struct Function;
struct GCReferenceList;
struct List;
struct ListData;
struct RawOutputPrefs;
struct TermMap;
struct StaticTypeQuery;
struct String;
struct StyledSource;
struct Term;
struct TermList;
struct Type;

typedef bool (*TermVisitor)(Term* term, caValue* context);

// Function-related typedefs:

#define CA_FUNCTION(fname) void fname(caStack* _stack)

typedef void (*EvaluateFunc)(caStack* stack);
typedef Type* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSource)(StyledSource* source, Term* term);
typedef bool (*CheckInvariants)(Term* term, std::string* output);
typedef void (*ReleaseFunc)(caValue* value);

const int MAX_INPUTS = 64;

// Garbage collection
typedef char GCColor;

// Assert macros
//
// ca_assert will (when enabled) call internal_error() if the condition is false.
//
// ca_test_assert does the same, but it is only enabled in "test" builds, so it's
// intended to be called in places that significantly harm performance.
#ifdef DEBUG

#ifdef CIRCA_TEST_BUILD

// Test build
#define ca_assert(x) circa::ca_assert_function((x), #x, __LINE__, __FILE__)
#define ca_test_assert(x) ca_assert(x)

#else

// Debug build
#define ca_assert(x) circa::ca_assert_function((x), #x, __LINE__, __FILE__)
#define ca_test_assert(x)

#endif

#else

// Release build
#define ca_assert(x)
#define ca_test_assert(x)

#endif

void ca_assert_function(bool result, const char* expr, int line, const char* file);

} // namespace circa

// Build flags

// ENABLE_DLL_LOADING - Enables functions that can load shared libraries using dlopen.
#ifndef CIRCA_ENABLE_DLL_LOADING

// Currently disabled
#define CIRCA_ENABLE_DLL_LOADING 0

#endif

// ENABLE_STDIN - Enables functions that read from STDIN, such as the interactive command line.
#ifndef CIRCA_ENABLE_STDIN
#define CIRCA_ENABLE_STDIN 1
#endif

// ENABLE_FILESYSTEM - Enables functions that look at the filesystem, including getcwd() and
// fopen()
#ifndef CIRCA_ENABLE_FILESYSTEM
#define CIRCA_ENABLE_FILESYSTEM 1
#endif

// ENABLE_THREADING - Enables functions that wrap around system threading utils
#ifndef CIRCA_ENABLE_THREADING
#define CIRCA_ENABLE_THREADING 1
#endif

// ENABLE_CL_TOOLS - Enables the separate command-line tools
#ifndef CIRCA_ENABLE_CL_TOOLS
#define CIRCA_ENABLE_CL_TOOLS 1
#endif
