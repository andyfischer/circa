// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "function.h"
#include "hashtable.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "loops.h"
#include "source_repro.h"
#include "stateful_code.h"
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

    // Add the zero block
    create_block_unevaluated(contents, "#zero");

    return iterator;
}

void add_implicit_placeholders(Term* forTerm)
{
    Block* contents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    Term* iterator = for_loop_get_iterator(contents);
    std::string iteratorName = iterator->name;

    std::vector<std::string> reboundNames;
    list_names_that_this_block_rebinds(contents, reboundNames);

    int inputIndex = 1;

    for (size_t i=0; i < reboundNames.size(); i++) {
        std::string const& name = reboundNames[i];
        if (name == listName)
            continue;
        if (name == iteratorName)
            continue;

        Term* original = find_name_at(forTerm, name.c_str());

        // The name might not be found, for certain parser errors.
        if (original == NULL)
            continue;

        Term* result = contents->get(name);

        // Create input_placeholder
        Term* input = apply(contents, FUNCS.input, TermList(), name.c_str());
        Type* type = find_common_type(original->type, result->type);
        set_declared_type(input, type);
        contents->move(input, inputIndex);

        set_input(forTerm, inputIndex, original);

        // Repoint terms to use our new input_placeholder
        for (BlockIterator it(contents); it.unfinished(); it.advance())
            remap_pointers_quick(*it, original, input);

        // Create output_placeholder
        Term* term = apply(contents, FUNCS.output, TermList(result), name.c_str());

        // Move output into the correct output slot
        contents->move(term, contents->length() - 1 - inputIndex);

        inputIndex++;
    }
}

void repoint_terms_to_use_input_placeholders(Block* contents)
{
    // Visit every term
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        // Visit every input
        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            Term* input = term->input(inputIndex);
            if (input == NULL)
                continue;
            
            // If the input is outside this block, then see if we have a named
            // input that could be used instead.
            if (input->owningBlock == contents || input->name == "")
                continue;

            Term* replacement = find_input_placeholder_with_name(contents, &input->nameValue);
            if (replacement == NULL)
                continue;

            set_input(term, inputIndex, replacement);
        }
    }
}

void list_names_that_must_be_looped(Block* contents, caValue* names)
{
    // Find all names within 'contents' that must be looped. A name must be looped when:
    //  (1) a term outside 'contents' has the name (the initial value).
    //  and (2) a term inside 'contents' actually uses the term that satisfies 1.
    //  and (3) a term inside 'contents' has the name (the 'modified' value)

    Value namesMap;
    set_hashtable(&namesMap);

    for (BlockInputIterator it(contents); it; ++it) {
        Term* input = it.currentInput();

        if (has_empty_name(input))
            continue;

        // Check condition (2). The input must be from outside 'contents'.
        if (input->owningBlock == contents || block_is_child_of(input->owningBlock, contents))
            continue;

        caValue* name = term_name(input);
        Term* local = find_local_name(contents, name);

        if (local != NULL)
            set_bool(hashtable_insert(&namesMap, name), true);
    }

    hashtable_get_keys(&namesMap, names);
    list_sort(names, NULL, NULL);
}

void update_looped_inputs(Block* contents)
{
    Value names;
    list_names_that_must_be_looped(contents, &names);

    // Terms 0 through N (where N = number of looped names) should be looped inputs.
    for (int i=0; i < list_length(&names); i++) {

        caValue* name = list_get(&names, i);
        Term* outside = find_name_at(contents->owningTerm, name);
        Term* inside = find_local_name(contents, name);

        Term* looped = contents->getSafe(i);
        if (looped == NULL || looped->function != FUNCS.looped_input) {
            looped = apply(contents, FUNCS.looped_input, TermList(outside, inside));
            move_to_index(looped, i);
        } else {
            set_inputs(looped, TermList(outside, inside));
        }
        rename(looped, as_cstring(name));

        // Now repoint inputs from the outside version to the looped input.
        for (BlockInputIterator it(contents); it; ++it)
            if (it.currentInput() == outside && it.currentTerm() != looped)
                set_input(it.currentTerm(), it.currentInputIndex(), looped);
    }

    // Delete any remaining looped terms.
    for (int i=list_length(&names); i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term->function == FUNCS.looped_input)
            erase_term(term);
    }
    remove_nulls(contents);
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
    // name, if it's been explicitly renamed.
    if (block->owningTerm->boolProp(sym_ModifyList, false)) {
        Term* term = block->get(iterator->name);
        if (term != NULL && term->function != FUNCS.get_index)
            return term;
    }

    // Otherwise, use the last expression as the output.
    return find_expression_for_implicit_output(block);
}

void finish_for_loop(Term* forTerm)
{
    Block* contents = nested_contents(forTerm);

    // Need to finish here to prevent error
    block_finish_changes(contents);

    // Add a a primary output
    Term* primaryOutput = apply(contents, FUNCS.output,
            TermList(loop_get_primary_result(contents)));
    primaryOutput->setBoolProp(sym_AccumulatingOutput, true);
    respecialize_type(primaryOutput);

    add_implicit_placeholders(forTerm);
    repoint_terms_to_use_input_placeholders(contents);

    update_extra_outputs(forTerm);

    block_finish_changes(contents);
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

Block* for_loop_get_zero_block(Block* contents)
{
    return contents->get("#zero")->contents();
}

void for_loop_remake_zero_block(Block* forContents)
{
    Block* zero = for_loop_get_zero_block(forContents);
    clear_block(zero);

    // Clone inputs
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(forContents, i);
        if (placeholder == NULL)
            break;
        Term* clone = append_input_placeholder(zero);
        rename(clone, &placeholder->nameValue);
        if (placeholder->boolProp(sym_State, false))
            clone->setBoolProp(sym_State, true);
    }

    Term* loopOutput = create_list(zero);

    // Clone outputs
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(forContents, i);
        if (placeholder == NULL)
            break;

        // Find the appropriate connection
        Term* result = find_local_name(zero, placeholder->name.c_str());

        if (i == 0)
            result = loopOutput;

        Term* clone = append_output_placeholder(zero, result);
        rename(clone, &placeholder->nameValue);
    }

    block_finish_changes(zero);
}

void start_for_loop(caStack* stack, bool enableLoopOutput)
{
    Frame* frame = top_frame(stack);
    Block* contents = frame->block;

    // Check if top frame actually contains a for-loop (it might be using the #zero block)
    if (!is_for_loop(contents))
        return;

    // Initialize the loop index
    set_int(frame_register(frame, for_loop_find_index(contents)), 0);

    if (enableLoopOutput) {
        // Initialize output value.
        caValue* outputList = stack_find_nonlocal(frame, contents->owningTerm);
        set_list(outputList, 0);
    }

    // Interpreter will run the contents of the block
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

void while_loop_finish_changes(Block* contents)
{
    update_looped_inputs(contents);
}

void while_formatSource(caValue* source, Term* term)
{
    Block* contents = nested_contents(term);
    format_name_binding(source, term);
    append_phrase(source, "while ", term, sym_Keyword);
    Term* conditionCheck = loop_find_condition_check(contents);
    format_source_for_input(source, conditionCheck, 0);
    append_phrase(source,
            term->stringProp(sym_Syntax_LineEnding, ""),
            term, tok_Whitespace);
    format_block_source(source, contents, term);
    append_phrase(source, term->stringProp(sym_Syntax_WhitespaceBeforeEnd, ""),
        term, tok_Whitespace);
}

void index_func_postCompile(Term* term)
{
    Term* enclosingLoop = find_enclosing_for_loop(term);
    if (enclosingLoop == NULL)
        return;
    Term* loop_index = for_loop_find_index(nested_contents(enclosingLoop));
    if (loop_index == NULL)
        return;
    set_input(term, 0, loop_index);
    set_input_hidden(term, 0, true);
}

void evaluate_index_func(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

void loop_setup_functions(Block* kernel)
{
    Term* index_func = import_function(kernel, evaluate_index_func, "index(int i :optional) -> int");
    block_set_post_compile_func(function_contents(index_func), index_func_postCompile);

    FUNCS.while_loop = import_function(kernel, NULL, "while()");
    block_set_format_source_func(function_contents(FUNCS.while_loop), while_formatSource);
}

} // namespace circa
