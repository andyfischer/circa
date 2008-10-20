#ifndef CIRCA_IMPORTING_INCLUDED
#define CIRCA_IMPORTING_INCLUDED

#include "common_headers.h"

#include "function.h"

namespace circa {

// Create a new Function with the given properties. Also binds the name in this branch.
Term* quick_create_function(Branch* code,
                            std::string name,
                            Function::EvaluateFunc evaluateFunc,
                            ReferenceList inputTypes,
                            Term* outputType);

// Similar to quick_create_function, except the function name, input types, and
// output type are all specified in 'header' using Circa function-declaration
// syntax.
//
// Example : "function do-something(int, string) -> int"
Term* import_c_function(Branch& branch, Function::EvaluateFunc func, std::string const& header);

}

#endif
