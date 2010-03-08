// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_EVAL_CONTEXT_INCLUDED
#define CIRCA_EVAL_CONTEXT_INCLUDED

#include "common_headers.h"
#include "references.h"

namespace circa {

struct EvalContext
{
    bool interruptBranch;

    // Error information:
    bool errorOccurred;
    Ref errorTerm;
    std::string errorMessage;

    EvalContext();
};

} // namespace circa

#endif
