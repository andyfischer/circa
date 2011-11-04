// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "refactoring.h"
#include "parser.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

Term* import_function(Branch* branch, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(branch, parser::function_decl, header);

    as_function(result)->evaluate = evaluate;
    return result;
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    ca_assert(is_function(function));
    as_function(function)->evaluate = evaluate;
}

void install_function(Branch* branch, const char* name, EvaluateFunc evaluate)
{
    Term* term = find_name(branch, name);
    as_function(term)->evaluate = evaluate;
}

Term* import_type(Branch* branch, Type* type)
{
    if (type->name == "")
        throw std::runtime_error("In import_type, type must have a name");

    Term* term = create_value(branch, &TYPE_T, type->name);
    set_type(term, type);
    return term;
}

} // namespace circa
