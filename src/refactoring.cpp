// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    if (!is_callable(function))
        throw std::runtime_error("Term "+function->name+" is not callable");

    // Check to specialize function
    function = specialize_function(function, term->inputs);

    term->function = function;

    // Check if we need to change the # of inputs
    if (!function_t::get_variable_args(function))
        term->inputs.resize(function_t::num_inputs(function));

    Term* newType = function_get_specialized_output_type(function, term);

    if (newType != ANY_TYPE)
        change_type(term, newType);
}

void unsafe_change_type(Term *term, Term *type)
{
    assert(type != NULL);

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    assert(term != NULL);
    assert(typeTerm != NULL);
    assert_type(typeTerm, TYPE_TYPE);

    Term* oldType = term->type;

    term->type = typeTerm;
    change_type(term, &as_type(typeTerm));

    if (is_branch(term))
        as_branch(term).owningTerm = term;

    if (oldType == typeTerm)
        return;

    assign_value_to_default(term);
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
            (term->owningBranch->get(term->name) == term)) {
        term->owningBranch->names.remove(term->name);
        term->name = "";
        term->owningBranch->bindName(term, name);
    }

    term->name = name;
}

void steal_term(Term* _term, Branch& newHome)
{
    Ref term = _term;

    if (term->owningBranch != NULL) {
        term->owningBranch->remove(term);
    }

    newHome.append(term);
}

void rewrite(Term* term, Term* function, RefList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);
    Term* outputType = function_t::get_output_type(function);

    if (function_t::get_specialize_type(function) != NULL)
        outputType = function_t::get_specialize_type(function)(term);

    change_type(term, outputType);
}

void rewrite_as_value(Branch& branch, int index, Term* type)
{
    while (index > branch.length())
        branch.append(NULL);

    if (index >= branch.length()) {
        create_value(branch, type);
    } else {
        Term* term = branch[index];

        change_function(term, VALUE_FUNC);
        change_type(term, type);
        set_inputs(term, RefList());
    }
}

void erase_term(Term* term)
{
    if (term->owningBranch != NULL)
        term->owningBranch->remove(term);
}

} // namespace circa
