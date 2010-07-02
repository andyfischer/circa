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

namespace circa {

struct Branch;
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

typedef bool (*TermVisitor)(Term* term, TaggedValue* context);

#define NEW_EVALUATE

// Function-related typedefs:
#ifdef NEW_EVALUATE

#define CA_FUNCTION(fname) \
    void fname(EvalContext* _circa_cxt, Term* _circa_caller, Term* _circa_func, \
            RefList const& _circa_inputs, TaggedValue* _circa_output)

typedef void (*EvaluateFunc)(EvalContext* cxt, Term* term, Term* func,
        RefList const& inputs, TaggedValue* output);
#else
typedef void (*EvaluateFunc)(EvalContext* context, Term* caller);

#define CA_FUNCTION(fname) \
    void fname(EvalContext* _circa_cxt, Term* _circa_caller)

#endif
typedef Term* (*SpecializeTypeFunc)(Term* caller);
typedef void (*FormatSource)(StyledSource* source, Term* term);
typedef bool (*CheckInvariants)(Term* term, std::string* output);

} // namespace circa
