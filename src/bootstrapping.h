// Copyright 2008 Andrew Fischer

#ifndef CIRCA__BOOTSTRAPPING__INCLUDED
#define CIRCA__BOOTSTRAPPING__INCLUDED

#include "common_headers.h"

#include "type.h"
#include "function.h"

namespace circa {

// Create a new Function with the given properties. Also binds the name.
Term* quick_create_function(Branch* code, string name, Function::EvaluateFunc evaluateFunc,
        ReferenceList inputTypes, Term* outputType);

} // namespace circa

#endif
