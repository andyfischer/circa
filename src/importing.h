#ifndef CIRCA_IMPORTING_INCLUDED
#define CIRCA_IMPORTING_INCLUDED

#include "common_headers.h"

#include "primitives.h"
#include "token_stream.h"

namespace circa {

// Create a Circa function, using the given C evaluation function, and
// a header in Circa-syntax.
//
// Example function header: "function do-something(int, string) -> int"
Term* import_function(Branch& branch, EvaluateFunc func, std::string const& header);

Term* import_function_overload(Term* overload, EvaluateFunc evaluate,
        std::string const& header);

Term* import_member_function(Term* type, EvaluateFunc evaluate,
        std::string const& headerText);

// Import the given value into this branch with the given name. We won't allocate
// a copy of this value, we'll use the address provided. The caller must ensure
// that this memory is accessible for the lifetime of this term.
Term* expose_value(Branch& branch, void* value, Term* type, std::string const& name="");

Term* expose_value(Branch& branch, int* value, std::string const& name="");
Term* expose_value(Branch& branch, float* value, std::string const& name="");

}

#endif
