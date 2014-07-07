// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "function.h"
#include "hashtable.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "loops.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

namespace circa {

Term* for_loop_get_iterator(Block* contents)
{
    for (int i=0; i < contents->length(); i++)
        if (contents->get(i)->function == FUNCS.get_index)
            return contents->get(i);
    return NULL;
}

Term* for_loop_find_index(Block* contents)
{
    return find_term_with_function(contents, FUNCS.loop_index);
}

const char* for_loop_get_iterator_name(Term* forTerm)
{
    Term* iterator = for_loop_get_iterator(nested_contents(forTerm));
    if (iterator == NULL)
        return "";

    return iterator->name.c_str();
}

Block* get_for_loop_outer_rebinds(Term* forTerm)
{
    Block* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

Term* start_building_for_loop(Term* forTerm, const char* iteratorName, Type* iteratorType)
{
    Block* contents = nested_contents(forTerm);

    // Add input placeholder for the list input
    Term* listInput = apply(contents, FUNCS.input, TermList());

    // Add loop_index()
    Term* index = apply(contents, FUNCS.loop_index, TermList(listInput));
    hide_from_source(index);

    // Add get_index to fetch the list's current element.
    Term* iterator = apply(contents, FUNCS.get_index, TermList(listInput, index), iteratorName);

    if (iteratorType == NULL)
        iteratorType = infer_type_of_get_index(forTerm->input(0));
    
    set_declared_type(iterator, iteratorType);
    hide_from_source(iterator);

    return iterator;
}

void list_names_that_must_be_looped(Block* contents, caValue* names)
{
    // Find all names within 'contents' that must be looped. A name must be looped when
    // a term inside the loop binds a name that was already used outside the loop.

    Value namesMap;
    set_hashtable(&namesMap);

    for (BlockIteratorFlat it(contents); it; ++it) {
        Term* term = it.current();

        if (has_empty_name(term))
            continue;

        Term* outsideName = find_name_at(contents->owningTerm, term_name(term));

        // Don't look at names outside the major block.
        if (outsideName != NULL && !is_under_same_major_block(term, outsideName))
            outsideName = NULL;

        if (outsideName != NULL)
            set_bool(hashtable_insert(&namesMap, term_name(term)), true);
    }

    hashtable_get_keys(&namesMap, names);
    list_sort(names, NULL, NULL);
}

void insert_looped_placeholders(Block* contents)
{
    Value names;
    list_names_that_must_be_looped(contents, &names);

    //set_input_placeholder_count(contents, hashtable_count(&names));
    //set_output_placeholder_count(contents, hashtable_count(&names));

    for (ListIterator it(&names); it; ++it) {
        Term* inputPlaceholder = append_input_placeholder(contents);
        Value* name = it.value();
        rename(inputPlaceholder, name);
        Term* outsideTerm = find_name_at(contents->owningTerm, name);
        Term* innerResult = find_local_name(contents, name);
        Term* outputPlaceholder = append_output_placeholder(contents, innerResult);
        rename(outputPlaceholder, name);

        set_inputs(inputPlaceholder, TermList(outsideTerm, outputPlaceholder));

        for (BlockInputIterator it(contents); it; ++it) {
            Term* term = it.currentTerm();
            if (it.currentInput() == outsideTerm && term != inputPlaceholder)
                set_input(term, it.currentInputIndex(), inputPlaceholder);
        }
    }
}

void list_names_that_should_be_used_as_minor_block_output(Block* block, caValue* names)
{
    Value namesMap;
    set_hashtable(&namesMap);
}

// Find the term that should be the 'primary' result for this loop.
Term* loop_get_primary_result(Block* block)
{
    Term* iterator = for_loop_get_iterator(block);

    // For a rebound list, use the last term that has the iterator's
    // name.
    if (block->owningTerm->boolProp(sym_ModifyList, false)) {
        Term* term = block->get(iterator->name);
        if (term != NULL)
            return term;
    }

    // Otherwise, use the last expression as the output.
    return find_expression_for_implicit_output(block);
}

void finish_while_loop(Block* block)
{
    block_finish_changes(block);

    // Add a a primary output
    apply(block, FUNCS.output, TermList(NULL));

    // Add looped_inputs
    insert_looped_placeholders(block);

    update_extra_outputs(block->owningTerm);
    update_for_control_flow(block);

    block_finish_changes(block);
}

void finish_for_loop(Term* forTerm)
{
    Block* block = nested_contents(forTerm);

    // Need to finish here to prevent error
    block_finish_changes(block);

    // Add a a primary output
    Term* primaryOutput = apply(block, FUNCS.output,
            TermList(loop_get_primary_result(block)));
    primaryOutput->setBoolProp(sym_AccumulatingOutput, true);
    respecialize_type(primaryOutput);

    insert_looped_placeholders(block);

    update_extra_outputs(forTerm);

    block_finish_changes(block);
}

Term* find_enclosing_for_loop(Term* location)
{
    while (location != NULL && location->function != FUNCS.for_func)
        location = get_parent_term(location);
    return location;
}

Block* find_enclosing_for_loop_contents(Term* term)
{
    Term* loop = find_enclosing_for_loop(term);
    if (loop == NULL)
        return NULL;
    return nested_contents(loop);
}

bool loop_produces_output_value(Term* forTerm)
{
    ca_assert(forTerm->function == FUNCS.for_func);
    return user_count(forTerm) > 0;
}
bool enclosing_loop_produces_output_value(Term* term)
{
    Term* enclosingForLoop = find_enclosing_for_loop(term);
    if (enclosingForLoop == NULL)
        return false;
    return loop_produces_output_value(enclosingForLoop);
}

void loop_add_condition_check(Block* caseBlock, Term* condition)
{
    apply(caseBlock, FUNCS.loop_condition_bool, TermList(condition));
}

Term* loop_find_condition_check(Block* block)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function == FUNCS.loop_condition_bool)
            return term;
    }
    return NULL;
}

Term* loop_find_condition(Block* block)
{
    Term* conditionCheck = loop_find_condition_check(block);
    if (conditionCheck != NULL)
        return conditionCheck->input(0);
    return NULL;
}

} // namespace circa
