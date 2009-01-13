#ifndef CIRCA_IMPORTING_INCLUDED
#define CIRCA_IMPORTING_INCLUDED

#include "common_headers.h"

#include "function.h"

namespace ast { class FunctionHeader; }

namespace circa {

// Create a Circa function, using the given C evaluation function, and
// a header in Circa-syntax.
//
// Example function header: "function do-something(int, string) -> int"
Term* import_function(Branch& branch, Function::EvaluateFunc func, std::string const& header);

Term* import_function(Branch& branch,
                        Function::EvaluateFunc func,
                        ast::FunctionHeader &header);
}

#endif
