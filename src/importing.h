// Copyright (c) Paul Hodge. See LICENSE file for license terms.

// importing.cpp : Functions to help import C functions into Circa.

#pragma once

#include "common_headers.h"

#include "function.h"

namespace circa {

// Create a Circa function, using the given C evaluation function, and
// a header in Circa-syntax.
//
// Example function header: "function do-something(int, string) -> int"
Term* import_function(Branch* branch, EvaluateFunc func, std::string const& header);

// Install an evaluate function into an existing function object.
void install_function(Term* function, EvaluateFunc evaluate);
Term* install_function(Branch* branch, const char* name, EvaluateFunc evaluate);

Term* import_type(Branch* branch, Type* type);

struct ImportRecord
{
    const char* functionName;
    EvaluateFunc evaluate;
};

void install_function_list(Branch* branch, const ImportRecord* list);

} // namespace circa
