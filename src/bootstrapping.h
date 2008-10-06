// Copyright 2008 Paul Hodge

#ifndef CIRCA_BOOTSTRAPPING_INCLUDED
#define CIRCA_BOOTSTRAPPING_INCLUDED

#include "common_headers.h"

#include "type.h"
#include "function.h"

namespace circa {

// Create a new Function with the given properties. Also binds the name.
Term* quick_create_function(Branch* code, std::string name, Function::EvaluateFunc evaluateFunc,
        ReferenceList inputTypes, Term* outputType);

} // namespace circa

#endif
