// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing.h"
#include "importing_macros.h"
#include "parser.h"
#include "names.h"
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

Term* install_function(Branch* branch, const char* name, EvaluateFunc evaluate)
{
    Term* term = find_name(branch, name_from_string(name));
    if (term == NULL) {
        std::string msg = "Name not found in install_function: ";
        msg += name;
        internal_error(msg.c_str());
    }
    as_function(term)->evaluate = evaluate;
    return term;
}

Term* import_type(Branch* branch, Type* type)
{
    Term* term = create_value(branch, &TYPE_T, name_to_string(type->name));
    set_type(term_value(term), type);
    return term;
}

void install_function_list(Branch* branch, const ImportRecord* list)
{
    while (list->functionName != NULL) {
        install_function(branch, list->functionName, list->evaluate);
        list++;
    }
}

} // namespace circa
