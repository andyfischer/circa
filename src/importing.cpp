// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Term* import_function(Branch& branch, EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(&branch, parser::function_decl, header);
    function_get_evaluate(result) = evaluate;
    return result;
}

Term* import_member_function(Term* type, EvaluateFunc evaluate, std::string const& header)
{
    Ref result = parser::compile(NULL, parser::function_decl, header);

    function_get_evaluate(result) = evaluate;
    as_type(type).addMemberFunction(result, function_get_name(result));
    return result;
}

Term* import_function_overload(Term* overload, EvaluateFunc evaluate, std::string const& header)
{
    return import_function(as_branch(overload), evaluate, header);
}

Term* expose_value(Branch* branch, void* value, Term* type, std::string const& name)
{
    Term* term = apply(branch, VALUE_FUNC, RefList(), name);
    change_type(term, type);
    term->value = value;
    term->boolProp("owned-value") = false;
    return term;
}

Term* expose_value(Branch* branch, int* value, std::string const& name)
{
    return expose_value(branch, value, INT_TYPE, name);
}
Term* expose_value(Branch* branch, float* value, std::string const& name)
{
    return expose_value(branch, value, FLOAT_TYPE, name);
}

} // namespace circa
