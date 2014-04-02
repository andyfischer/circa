// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#define __STDC_CONSTANT_MACROS

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
#include <iostream>
#include <string>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>

#define EXPORT extern "C"

#include "circa/circa.h"
#include "circa/objects.h"
#include "names_builtin.h"

namespace circa {

typedef long long int int64;
typedef long long unsigned int uint64;
typedef uint16_t u16;
typedef uint32_t u32;

struct Block;
struct CastResult;
struct CircaObject;
struct Frame;
struct Stack;
struct FeedbackOperation;
struct FileWatch;
struct FileWatchWorld;
struct GCReferenceList;
struct ListData;
struct LibuvWorld;
struct NativePatchWorld;
struct NativePatch;
struct NativePatchFunction;
struct RawOutputPrefs;
struct StaticTypeQuery;
struct Term;
struct TermList;
struct TermMap;
struct Type;

typedef bool (*TermVisitor)(Term* term, caValue* context);
typedef int Symbol;

// Function-related typedefs:

typedef void (*EvaluateFunc)(caStack* stack);
typedef Type* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSourceFunc)(caValue* source, Term* term);
typedef void (*ReleaseFunc)(caValue* value);
typedef void (*PostCompileFunc)(Term*);

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
  #ifdef DEBUG
    #define CIRCA_ENABLE_PERF_STATS 1
  #else
    #define CIRCA_ENABLE_PERF_STATS 0
  #endif
#endif

// ENABLE_HEAP_DEBUGGING
//
// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. Has a
// large performance penalty.

#ifndef CIRCA_ENABLE_HEAP_DEBUGGING
 #ifdef CIRCA_TEST_BUILD
  #define CIRCA_ENABLE_HEAP_DEBUGGING 1
 #else
  #define CIRCA_ENABLE_HEAP_DEBUGGING 0
 #endif
#endif

// ENABLE_SNEAKY_EQUALS - When enabled, equals() is allowed to combine the
// internal representation of values (when it's correct to do so).
#define CIRCA_ENABLE_SNEAKY_EQUALS 1

#define CIRCA_ENABLE_INLINE_DYNAMIC_METHOD_CACHE 1
