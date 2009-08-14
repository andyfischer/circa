// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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
    Ref result = parser::compile(NULL, parser::function_decl, header);

    function_t::get_evaluate(result) = evaluate;
    as_type(type).addMemberFunction(result, function_t::get_name(result));
    return result;
}

Term* import_function_overload(Term* overload, EvaluateFunc evaluate, std::string const& header)
{
    return import_function(as_branch(overload), evaluate, header);
}

} // namespace circa
