// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "builtins.h"
#include "function.h"
#include "heap_debugging.h"
#include "introspection.h"
#include "list_shared.h"
#include "names.h"
#include "static_checking.h"
#include "term.h"
#include "type.h"

#include "refactoring.h"

namespace circa {

void change_function(Term* term, Term* function)
{
    if (term->function == function)
        return;

    Term* previousFunction = term->function;

    term->function = function;

    possibly_prune_user_list(term, previousFunction);
    respecialize_type(term);

    // Don't append user for certain functions. Need to make this more robust.
    if (function != NULL
            && function != VALUE_FUNC
            && function != INPUT_PLACEHOLDER_FUNC) {
        append_user(term, function);
    }
}

void unsafe_change_type(Term *term, Type *type)
{
    ca_assert(type != NULL);

    term->type = type;
}

void change_declared_type(Term *term, Type *newType)
{
    ca_assert(term != NULL);
    ca_assert(newType != NULL);

    if (term->type == newType)
        return;

    term->type = newType;

    set_null((TaggedValue*) term);

    // TODO: Don't call change_type here
    change_type(term, newType);

    // TODO: Use update_cascades to update inferred type on all users.
}

void respecialize_type(Term* term)
{
    if (SHUTTING_DOWN)
        return;

    Type* outputType = derive_specialized_output_type(term->function, term);
    if (outputType != term->type)
        change_declared_type(term, outputType);
}

void specialize_type(Term *term, Type *type)
{
    if (term->type == type)
        return;

    ca_assert(term->type == &ANY_T);

    change_declared_type(term, type);
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

void rewrite(Term* term, Term* function, TermList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);
    Type* outputType = function_get_output_type(function, 0);

    Function* attrs = get_function_attrs(function);

    if (attrs->specializeType != NULL)
        outputType = attrs->specializeType(term);

    change_declared_type(term, outputType);
}

void rewrite_as_value(Branch* branch, int index, Type* type)
{
    while (index > branch->length())
        branch->append(NULL);

    if (index >= branch->length()) {
        create_value(branch, type);
    } else {
        Term* term = branch->get(index);

        change_function(term, VALUE_FUNC);
        change_declared_type(term, type);
        set_inputs(term, TermList());
    }
}

void remove_term(Term* term)
{
    assert_valid_term(term);

    int index = term->index;
    Branch* branch = term->owningBranch;

    erase_term(term);

    for (int i=index; i < branch->_terms.length()-1; i++) {
        branch->_terms.setAt(i, branch->_terms[i+1]);
        if (branch->_terms[i] != NULL)
            branch->_terms[i]->index = i;
    }
    branch->_terms.resize(branch->_terms.length()-1);

    if (is_list(&branch->pendingUpdates))
        list_remove_index(&branch->pendingUpdates, index);
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
    
    Type::RemapPointers remapPointers = term->type->remapPointers;

    // Remap on value
    if ((term->value_data.ptr != NULL)
            && term->type != NULL
            && (remapPointers)) {

        remapPointers(term, map);
    }

    // This code once called remap on term->properties

    // Remap inside nestedContents
    if (has_nested_contents(term))
        nested_contents(term)->remapPointers(map);
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

void remap_pointers(Branch* branch, Term* original, Term* replacement)
{
    TermMap map;
    map[original] = replacement;

    for (int i=0; i < branch->length(); i++) {
        if (branch->get(i) == NULL) continue;
        remap_pointers(branch->get(i), map);
    }
}

} // namespace circa
