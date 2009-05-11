// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Term* import_function(Branch& branch, Function::EvaluateFunc evaluate, std::string const& header)
{
    Term* result = create_empty_function(branch, header);
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

Term* expose_value(Branch& branch, int* value, std::string const& name)
{
    apply(&branch, VALUE_FUNC, RefList(), name);


}


} // namespace circa
