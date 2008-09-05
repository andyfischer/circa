#ifndef CIRCA__BOOTSTRAPPING__INCLUDED
#define CIRCA__BOOTSTRAPPING__INCLUDED

#include "common_headers.h"

#include "type.h"
#include "function.h"

namespace circa {

// Create a new Type with the given properties. Also binds the name.
Term* quick_create_type(
        Branch* branch,
        std::string name,
        Type::AllocFunc allocFunc,
        Type::DeallocFunc deallocFunc,
        Type::DuplicateFunc duplicateFunc,
        Type::ToStringFunc toStringFunc = NULL);

// Create a new Function with the given properties. Also binds the name.
Term* quick_create_function(Branch* code, string name, Function::EvaluateFunc evaluateFunc,
        TermList inputTypes, Term* outputType);

void initialize_bootstrapped_code(Branch* kernel);

} // namespace circa

#endif
