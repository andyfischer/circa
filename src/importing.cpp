// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "refactoring.h"
#include "parser.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

CA_FUNCTION(empty_evaluate_no_touch_output)
{
}

Term* import_function(Branch& branch, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(branch, parser::function_decl, header);

    if (evaluate == NULL)
        evaluate = empty_evaluate_function;

    function_t::get_evaluate(result) = evaluate;
    return result;
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    ca_assert(is_function(function));
    function_t::get_evaluate(function) = evaluate;

    for (int i=0; i < function->users.length(); i++) {
        Term* user = function->users[i];
        update_cached_evaluate_func(user);
    }
}

Term* import_type(Branch& branch, Type* type)
{
    if (type->name == "")
        throw std::runtime_error("In import_type, type must have a name");

    Term* term = create_value(branch, TYPE_TYPE, type->name);
    set_type(term, type);
    return term;
}

} // namespace circa
