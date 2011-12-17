// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

// Circa headers
#include "symbols.h"

#define export_func extern "C"

namespace circa {

typedef int symbol;

struct Branch;
struct BrokenLinkList;
struct CastResult;
struct CircaObject;
struct Dict;
struct EvalContext;
struct FeedbackOperation;
struct Function;
struct GCReferenceList;
struct List;
struct ListData;
struct RawOutputPrefs;
struct TermMap;
struct StaticTypeQuery;
struct StyledSource;
struct TaggedValue;
struct Term;
struct TermList;
struct Type;

typedef Term* TermPtr;
typedef bool (*TermVisitor)(Term* term, TaggedValue* context);

// Variant value used in many places, including in TaggedValue:
union VariantValue {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

// Function-related typedefs:

#define CA_FUNCTION(fname) \
    void fname(circa::EvalContext* _cxt, circa::ListData* _ins, circa::ListData* _outs)

typedef void (*EvaluateFunc)(EvalContext* cxt, ListData* inputs, ListData* outputs);
typedef Type* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSource)(StyledSource* source, Term* term);
typedef bool (*CheckInvariants)(Term* term, std::string* output);

// Garbage collection
typedef char GCColor;

// Used in evaluation functions:
int list_size(ListData* data);

// Possibly enable ca_assert and/or ca_test_assert. When enabled, ca_assert will
// call internal_error() if the condition is false.
//
// ca_test_assert does the same thing, but it is only enabled in "test" builds, so it's
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
