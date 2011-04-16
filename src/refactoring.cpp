// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "debug_valid_objects.h"
#include "errors.h"
#include "function.h"
#include "names.h"
#include "static_checking.h"
#include "term.h"
#include "type.h"

#include "refactoring.h"

namespace circa {

EvaluateFunc derive_evaluate_func(Term* term)
{
    if (!FINISHED_BOOTSTRAP && term->function == VALUE_FUNC)
        return value_function::evaluate;

    if (term->function == NULL)
        return empty_evaluate_function;

    if (!is_function(term->function))
        return empty_evaluate_function;

    return function_t::get_evaluate(term->function);
}

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    Term* previousFunction = term->function;

    term->function = function;

    term->evaluateFunc = derive_evaluate_func(term);

    possibly_prune_user_list(term, previousFunction);
    respecialize_type(term);

    //if (function != NULL)
    //    function->users.appendUnique(term);
}

void unsafe_change_type(Term *term, Term *type)
{
    ca_assert(type != NULL);

    term->type = type;
}

void change_type(Term *term, Term *typeTerm)
{
    ca_assert(term != NULL);
    ca_assert(typeTerm != NULL);
    ca_assert(typeTerm->type == TYPE_TYPE);

    Term* oldType = term->type;

    term->type = typeTerm;
    change_type(term, unbox_type(typeTerm));

    if (oldType == typeTerm)
        return;

    // Cascade type inference
    for (int user=0; user < term->users.length(); user++) {
        Term* userTerm = term->users[user];
        debug_assert_valid_object(userTerm, TERM_OBJECT);
        respecialize_type(userTerm);
    }
}

void respecialize_type(Term* term)
{
    Term* outputType = derive_specialized_output_type(term->function, term);
    if (outputType != term->type)
        change_type(term, outputType);
}

void specialize_type(Term *term, Term *type)
{
    if (term->type == type)
        return;

    ca_assert(term->type == ANY_TYPE);

    change_type(term, type);
}

void rename(Term* term, std::string const& name)
{
    if (term->name == name)
        return;

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
    Term* outputType = function_get_output_type(function, 0);

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
    assert_valid_term(term);

    set_inputs(term, RefList());
    clear_branch(&term->nestedContents);

    // for each user, clear that user's input list of this term
    clear_from_users_inputs(term);

    if (term->owningBranch != NULL)
        term->owningBranch->remove(term);
}

} // namespace circa
