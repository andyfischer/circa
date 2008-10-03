#ifndef CIRCA__IMPORTING__INCLUDED
#define CIRCA__IMPORTING__INCLUDED

#include "common_headers.h"

#include "function.h"

namespace circa {

Term* import_c_function(Branch& branch, Function::EvaluateFunc func, std::string const& header);

}

#endif
