// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

CA_FUNCTION(empty_evaluate_function) {}

Term* import_function(Branch& branch, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(&branch, parser::function_decl, header);

    if (evaluate == NULL)
        evaluate = empty_evaluate_function;

    function_t::get_evaluate(result) = evaluate;
    return result;
}

Term* import_member_function(Type* type, EvaluateFunc evaluate, std::string const& header)
{
    Ref result = parser::compile(&type->memberFunctions,
            parser::function_decl, header);

    function_t::get_evaluate(result) = evaluate;
    return result;
}

Term* import_member_function(Term* type, EvaluateFunc evaluate, std::string const& header)
{
    return import_member_function(&as_type(type), evaluate, header);
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    ca_assert(is_function(function));
    function_t::get_evaluate(function) = evaluate;
}

Term* import_type(Branch& branch, Type* type)
{
    if (type->name == "")
        throw std::runtime_error("In import_type, type must have a name");

    Term* term = create_value(branch, TYPE_TYPE, type->name);
    type_t::copy(type, term);
    return term;
}

} // namespace circa
