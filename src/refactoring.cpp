// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "function.h"
#include "heap_debugging.h"
#include "introspection.h"
#include "list_shared.h"
#include "names.h"
#include "subroutine.h"
#include "stateful_code.h"
#include "static_checking.h"
#include "term.h"
#include "type.h"

#include "refactoring.h"

namespace circa {

void rewrite(Term* term, Term* function, TermList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);
    Type* outputType = function_get_output_type(function, 0);

    Function* attrs = as_function(function);

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

        change_function(term, BUILTIN_FUNCS.value);
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

void remap_pointers_quick(Term* term, Term* old, Term* newTerm)
{
    for (int i=0; i < term->numInputs(); i++)
        if (term->input(i) == old)
            set_input(term, i, newTerm);
}

void remap_pointers_quick(Branch* branch, Term* old, Term* newTerm)
{
    for (int i=0; i < branch->length(); i++)
        remap_pointers_quick(branch->get(i), old, newTerm);
}

void remap_pointers(Term* term, TermMap const& map)
{
    assert_valid_term(term);

    // make sure this map doesn't try to remap NULL, because such a thing
    // would almost definitely lead to errors.
    ca_assert(!map.contains(NULL));

    for (int i=0; i < term->numInputs(); i++)
        set_input(term, i, map.getRemapped(term->input(i)));

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
