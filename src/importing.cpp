// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

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

Term* import_function_overload(Term* overload, EvaluateFunc evaluate, std::string const& header)
{
    return import_function(as_branch(overload), evaluate, header);
}

void install_function(Term* function, EvaluateFunc evaluate)
{
    assert(is_function(function));
    function_t::get_evaluate(function) = evaluate;
}

void shallow_assign(Term* a, Term* b)
{
    b->value = a->value;
}

bool shallow_equals(Term* a, Term* b)
{
    return a->value == b->value;
}

void zero_alloc(Term *type, Term* t)
{
    t->value = 0;
}

} // namespace circa
