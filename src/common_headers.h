// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#ifdef WINDOWS

#undef max
#undef min

#define _USE_MATH_DEFINES
#include <math.h>
#include <direct.h> 

#endif // WINDOWS

#include <algorithm>
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

namespace circa {

struct Branch;
struct Dict;
struct EvalContext;
struct FeedbackOperation;
struct FunctionAttrs;
struct List;
struct PathExpression;
struct RawOutputPrefs;
struct Ref;
struct RefList;
struct ReferenceMap;
struct StaticTypeQuery;
struct StyledSource;
struct TaggedValue;
struct Term;
struct Type;
struct TypeRef;

namespace bytecode {
    struct BytecodeData;
    struct CallOperation;
    struct WriteContext;
}

typedef bool (*TermVisitor)(Term* term, TaggedValue* context);

// Function-related typedefs:

#define CA_FUNCTION(fname) \
    void fname(circa::EvalContext* _circa_cxt, \
            List* _circa_stack, \
            bytecode::CallOperation* _circa_op)

typedef void (*EvaluateFunc)(EvalContext* cxt, List* stack,
        bytecode::CallOperation* call);
typedef Term* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSource)(StyledSource* source, Term* term);
typedef bool (*CheckInvariants)(Term* term, std::string* output);

// ca_assert results in a call to internal_error. On failure, this function may throw
// an exception or trigger a debugger breakpoint. But either way, execution will
// be interrupted. ca_assert_function is defined in errors.cpp
#define ca_assert(x) circa::ca_assert_function((x), #x, __LINE__, __FILE__)
void ca_assert_function(bool result, const char* expr, int line, const char* file);

} // namespace circa
