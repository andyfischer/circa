// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#ifdef _MSC_VER
// Special handling for Windows

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h> 

#endif // Windows

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
#include "circa/objects.h"
#include "names_builtin.h"

namespace circa {

typedef long long int int64;
typedef long long unsigned int uint64;

struct Block;
struct CastResult;
struct CircaObject;
struct Dict;
struct Frame;
struct Stack;
struct FeedbackOperation;
struct FileWatch;
struct FileWatchWorld;
struct Function;
struct GCReferenceList;
struct List;
struct ListData;
struct NativePatchWorld;
struct NativePatch;
struct NativePatchFunction;
struct RawOutputPrefs;
struct StaticTypeQuery;
struct String;
struct Term;
struct TermList;
struct TermMap;
struct Type;

typedef bool (*TermVisitor)(Term* term, caValue* context);
typedef int Name;

// Function-related typedefs:

#define CA_FUNCTION(fname) void fname(caStack* _stack)

typedef void (*EvaluateFunc)(caStack* stack);
typedef Type* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSource)(caValue* source, Term* term);
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

// ENABLE_FILESYSTEM - Enables functions that look at the filesystem, including getcwd() and
// fopen()
#ifndef CIRCA_ENABLE_FILESYSTEM
#define CIRCA_ENABLE_FILESYSTEM 1
#endif

// ENABLE_THREADING - Enables functions that wrap around system threading utils
#ifndef CIRCA_ENABLE_THREADING
#define CIRCA_ENABLE_THREADING 1
#endif

// ENABLE_LOGGING - Enables internal logging (disabled by default)
#ifndef CIRCA_ENABLE_LOGGING
#define CIRCA_ENABLE_LOGGING 0
#endif

// ENABLE_PERF_STATS - Enables tracking of internal performance metrics
#ifndef CIRCA_ENABLE_PERF_STATS
#define CIRCA_ENABLE_PERF_STATS 1
#endif

// ENABLE_HEAP_DEBUGGING
//
// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. Has a
// large performance penalty.

#ifdef CIRCA_TEST_BUILD

#define CIRCA_ENABLE_HEAP_DEBUGGING 1

#else

#define CIRCA_ENABLE_HEAP_DEBUGGING 0

#endif

// Trigger an assert when internal_error is called. If this is off, the alternative is
// that an exception is thrown.
#define CIRCA_ASSERT_ON_ERROR 1

#define CIRCA_THROW_ON_ERROR !CIRCA_ASSERT_ON_ERROR
