// Copyright 2008 Paul Hodge

#include "common_headers.h"
#include "circa.h"
#include "values.h"

namespace circa {

void alloc_value(Term* term)
{
    term->value = alloc_from_type(term->type);
    update_owner(term);
}

void dealloc_value(Term* term)
{
    if (term->value == NULL)
        return;

    if (term->type == NULL)
        return;

    if (term->type->value == NULL)
        throw std::runtime_error("type is undefined");

    if (term->ownsValue) {
        if (as_type(term->type).dealloc == NULL)
            throw std::runtime_error("type " + as_type(term->type).name
                + " has no dealloc function");

        as_type(term->type).dealloc(term->value);
    }

    term->value = NULL;
}

void recycle_value(Term* source, Term* dest)
{
    assert_type(source, dest->type);

    // Usually don't steal. Later, as an optimization, we will sometimes steal.
    bool steal = false;

    // Steal if this type has no copy function
    if (as_type(source->type).copy == NULL)
        steal = true;

    if (steal) {
        steal_value(source, dest);
    } else {
        copy_value(source, dest);
    }
}

void copy_value(Term* source, Term* dest)
{
    // Temp: Do a type specialization if dest has type 'any'.
    // This should be removed once type inference rules are smarter.
    if (dest->type == ANY_TYPE)
        specialize_type(dest, source->type);

    assert_type(source, dest->type);

    if (dest->value == NULL)
        alloc_value(dest);

    Type::CopyFunc copy = as_type(source->type).copy;

    if (copy == NULL)
        throw std::runtime_error(std::string("type ") + as_type(source->type).name
                + " has no copy function");


    copy(source, dest);
}

void copy_value_but_dont_copy_inner_branch(Term* source, Term* dest)
{
    // Special case: functions. Need to copy a bunch of data, but not the
    // subroutineBranch.

    assert_type(source, dest->type);

    /*if (source->type == FUNCTION_TYPE) {
        Function::copyExceptBranch(source,dest);
        return;
    }
    */
    
    if (has_inner_branch(dest))
        return;

    copy_value(source, dest);
}

void steal_value(Term* source, Term* dest)
{
    assert_type(source, dest->type);

    // if 'dest' has a value, delete it
    dealloc_value(dest);

    dest->value = source->value;

    source->value = NULL;
    source->needsUpdate = true;

    update_owner(dest);
}

void update_owner(Term* term)
{
    if (term->value == NULL)
        return;

    Type &type = as_type(term->type);

    if (type.updateOwner == NULL)
        return;

    type.updateOwner(term);
}

bool values_equal(Term* a, Term* b)
{
    if (a->type != b->type)
        return false;

    return as_type(a->type).equals(a,b);
}

Term* create_value(Branch* branch, Term* type, std::string const& name)
{
    assert(type != NULL);
    if (branch == NULL)
        assert(name == "");
    assert(is_type(type));

    Term *var_function = get_value_function(type);
    Term *term = create_term(branch, var_function, RefList());

    alloc_value(term);

    term->stealingOk = false;
    //term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;

    if (name != "")
        branch->bindName(term, name);

    return term;
}

Term* create_value(Branch* branch, std::string const& typeName, std::string const& name)
{
    Term *type = NULL;

    type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error(std::string("Couldn't find type: ")+typeName);

    return create_value(branch, type, name);
}

Term* import_value(Branch& branch, Term* type, void* initialValue, std::string const& name)
{
    assert(type != NULL);
    Term *var_function = get_value_function(type);
    Term *term = create_term(&branch, var_function, RefList());

    term->value = initialValue;
    term->ownsValue = false;
    term->stealingOk = false;

    if (name != "")
        branch.bindName(term, name);

    return term;
}

Term* import_value(Branch& branch, std::string const& typeName, void* initialValue, std::string const& name)
{
    Term* type = find_named(&branch, typeName);

    if (type == NULL)
        throw std::runtime_error("Couldn't find type: "+typeName);

    return import_value(branch, type, initialValue, name);
}

Term* string_value(Branch& branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(&branch, STRING_TYPE);
    as_string(term) = s;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* int_value(Branch& branch, int i, std::string const& name)
{
    Term* term = create_value(&branch, INT_TYPE);
    as_int(term) = i;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* float_value(Branch& branch, float f, std::string const& name)
{
    Term* term = create_value(&branch, FLOAT_TYPE);
    as_float(term) = f;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* bool_value(Branch& branch, bool b, std::string const& name)
{
    Term* term = create_value(&branch, BOOL_TYPE);
    as_bool(term) = b;
    if (name != "")
        branch.bindName(term, name);
    return term;
}

Term* create_alias(Branch& branch, Term* term)
{
    return eval_function(branch, ALIAS_FUNC, RefList(term));
}

} // namespace circa
