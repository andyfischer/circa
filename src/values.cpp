// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

void alloc_value(Term* term)
{
    term->value = alloc_from_type(term->type);
}

void dealloc_value(Term* term)
{
    if (!is_value_alloced(term))
        return;

    if (term->type == NULL)
        return;

    if (!is_value_alloced(term->type)) {
        std::cout << "warning in dealloc_value, type is undefined" << std::endl;
        term->value = NULL;
        return;
    }

    if (as_type(term->type).dealloc == NULL)
        throw std::runtime_error("type "+as_type(term->type).name+" has no dealloc function");

    as_type(term->type).dealloc(term->value);

    term->value = NULL;
}

bool is_value_alloced(Term* term)
{
    // Future: return true for in-place values, when those are implemented.
    return term->value != NULL;
}

void assign_value(Term* source, Term* dest)
{
    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    assert(value_fits_type(source, dest->type));

    if (!is_value_alloced(dest))
        alloc_value(dest);

    Type::AssignFunc assign = as_type(source->type).assign;

    if (assign == NULL)
        throw std::runtime_error("type "+as_type(source->type).name+" has no assign function");

    assign(source, dest);
}

void assign_value_but_dont_copy_inner_branch(Term* source, Term* dest)
{
    // Do a type specialization if dest has type 'any'.
    // This might be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    assert(value_fits_type(source, dest->type));

    // Special case for functions
    if (source->type == FUNCTION_TYPE) {
        Function::copyExceptBranch(source,dest);
        return;
    }
    
    // Otherwise, do nothing for types with branches
    if (has_inner_branch(dest))
        return;

    assign_value(source, dest);
}

Term* import_value(Branch* branch, Term* type, void* initialValue, std::string const& name)
{
    assert(type != NULL);
    Term *var_function = get_value_function(type);
    Term *term = create_term(branch, var_function, RefList());

    term->value = initialValue;
    //term->ownsValue = false;
    term->stealingOk = false;

    if (name != "" && branch != NULL)
        branch->bindName(term, name);

    return term;
}

Term* import_value(Branch* branch, std::string const& typeName, void* initialValue, std::string const& name)
{
    Term* type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error("Couldn't find type: "+typeName);

    return import_value(branch, type, initialValue, name);
}

Term* string_value(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, STRING_TYPE, name);
    as_string(term) = s;
    return term;
}

Term* int_value(Branch* branch, int i, std::string const& name)
{
    Term* term = create_value(branch, INT_TYPE, name);
    as_int(term) = i;
    return term;
}

Term* float_value(Branch* branch, float f, std::string const& name)
{
    Term* term = create_value(branch, FLOAT_TYPE, name);
    as_float(term) = f;
    return term;
}

Term* bool_value(Branch* branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, BOOL_TYPE, name);
    as_bool(term) = b;
    return term;
}

} // namespace circa
