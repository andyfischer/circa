// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

void change_function(Term* term, Term* newFunction)
{
    term->function = newFunction;

    Function& func = as_function(newFunction);
    change_type(term, func.outputType);
}

void unsafe_change_type(Term *term, Term *type)
{
    assert(type != NULL);

    if (!is_value_alloced(term)) {
        change_type(term, type);
        return;
    }

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    assert_type(typeTerm, TYPE_TYPE);

    if (term->type == typeTerm)
        return;

    if (term->type != NULL)
        dealloc_value(term);

    term->type = typeTerm;
}

void specialize_type(Term *term, Term *type)
{
    if (term->type == type)
        return;

    assert_type(term, ANY_TYPE);

    change_type(term, type);
}

void rename(Term* term, std::string const& name)
{
    if ((term->owningBranch != NULL) &&
            (term->owningBranch->getNamed(term->name) == term)) {
        term->owningBranch->names.remove(term->name);
        term->name = "";
        term->owningBranch->bindName(term, name);
    }

    term->name = name;
}

} // namespace circa
