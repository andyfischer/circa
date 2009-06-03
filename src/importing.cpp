// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Term* import_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& header)
{
    Term* result = parser::compile(&branch, parser::function_from_header, header);
    as_function(result).evaluate = evaluate;
    return result;
}

Term* import_member_function(Term* type, Function::EvaluateFunc evaluate, std::string const& header)
{
    Ref result = parser::compile(NULL, parser::function_from_header, header);

    as_function(result).evaluate = evaluate;
    as_type(type).addMemberFunction(result, as_function(result).name);
    return result;
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
