// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "debug_valid_objects.h"
#include "errors.h"
#include "function.h"
#include "introspection.h"
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

void update_cached_evaluate_func(Term* term)
{
    term->evaluateFunc = derive_evaluate_func(term);
}

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    Term* previousFunction = term->function;

    term->function = function;

    update_cached_evaluate_func(term);

    possibly_prune_user_list(term, previousFunction);
    respecialize_type(term);

    // Don't append user for certain functions. Need to make this more robust.
    if (function != NULL
            && function != VALUE_FUNC
            && function != INPUT_PLACEHOLDER_FUNC) {
        append_user(term, function);
    }
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
    if (SHUTTING_DOWN)
        return;

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
    Term* term = _term;

    if (term->owningBranch != NULL) {
        term->owningBranch->remove(term);
    }

    newHome.append(term);
}

void rewrite(Term* term, Term* function, TermList const& inputs)
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
        set_inputs(term, TermList());
    }
}

void remove_term(Term* term)
{
    term->owningBranch->remove(term);
}

void remap_pointers(Term* term, TermMap const& map)
{
    assert_valid_term(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    ca_assert(!map.contains(NULL));

    for (int i=0; i < term->numInputs(); i++)
        set_input2(term, i, map.getRemapped(term->input(i)), term->inputInfo(i)->outputIndex);

    term->function = map.getRemapped(term->function);

    // TODO, call changeType if our type is changed
    // This was implemented once, and it caused spurious crash bugs
    // Term* newType = map.getRemapped(term->type);
    
    Type::RemapPointers remapPointers = type_t::get_remap_pointers_func(term->type);

    // Remap on value
    if ((term->value_data.ptr != NULL)
            && term->type != NULL
            && (remapPointers)) {

        remapPointers(term, map);
    }

    // This code once called remap on term->properties

    // Remap inside nestedContents
    term->nestedContents.remapPointers(map);
}

void remap_pointers(Term* term, Term* original, Term* replacement)
{
    assert_valid_term(term);
    assert_valid_term(original);
    ca_assert(original != NULL);

    TermMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void remap_pointers(Branch& branch, Term* original, Term* replacement)
{
    TermMap map;
    map[original] = replacement;

    for (int i=0; i < branch.length(); i++) {
        if (branch[i] == NULL) continue;
        remap_pointers(branch[i], map);
    }
}

} // namespace circa
