// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

Term* import_function(Branch& branch, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(&branch, parser::function_decl, header);
    function_t::get_evaluate(result) = evaluate;
    return result;
}

Term* import_member_function(Term* type, EvaluateFunc evaluate, std::string const& header)
{
    Ref result = parser::compile(&type_t::get_member_functions(type),
            parser::function_decl, header);

    function_t::get_evaluate(result) = evaluate;
    return result;
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    assert(is_function(function));
    function_t::get_evaluate(function) = evaluate;
}

Term* import_type(Branch& branch, Type* type)
{
    if (type->name == "")
        throw std::runtime_error("In import_type, type must have a name");

    Term* term = create_value(branch, TYPE_TYPE, type->name);
    set_pointer(term->value, &as_type(TYPE_TYPE), type);
    return term;
}

} // namespace circa
