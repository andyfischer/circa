// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "function.h"
#include "kernel.h"
#include "importing.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "loops.h"

namespace circa {

void for_loop_fix_state_input(Block* contents);

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
Term* for_loop_find_output_index(Block* contents)
{
    return find_term_with_function(contents, FUNCS.loop_output_index);
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
    
    change_declared_type(iterator, iteratorType);
    hide_from_source(iterator);

    // Add the zero block
    create_block_unevaluated(contents, "#zero");

    // Add an loop output index
    apply(contents, FUNCS.loop_output_index, TermList());

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
        change_declared_type(input, type);
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

// Find the term that should be the 'primary' result for this loop.
Term* loop_get_primary_result(Block* block)
{
    Term* iterator = for_loop_get_iterator(block);

    // For a rebound list, use the last term that has the iterator's
    // name, even if it's the iterator itself.
    if (block->owningTerm->boolProp("modifyList", false))
        return block->get(iterator->name);

    // Otherwise, use the last expression as the output.
    return find_last_non_comment_expression(block);
}

void finish_for_loop(Term* forTerm)
{
    Block* contents = nested_contents(forTerm);

    // Need to finish here to prevent error
    block_finish_changes(contents);

    // Add a a primary output
    Term* primaryOutput = apply(contents, FUNCS.output,
            TermList(loop_get_primary_result(contents)));
    primaryOutput->setBoolProp("accumulatingOutput", true);
    respecialize_type(primaryOutput);

    // pack_any_open_state_vars(contents);
    for_loop_fix_state_input(contents);
    check_to_add_state_output_placeholder(contents);

    add_implicit_placeholders(forTerm);
    repoint_terms_to_use_input_placeholders(contents);

    check_to_insert_implicit_inputs(forTerm);
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

bool is_for_loop(Block* block)
{
    if (block->owningTerm == NULL)
        return false;
    if (FUNCS.for_func == NULL)
        return false;
    return block->owningTerm->function == FUNCS.for_func;
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

void for_loop_fix_state_input(Block* contents)
{
    // This function will look at the state access inside for-loop contents.
    // If there's state, the default building functions will have created
    // terms that look like this:
    //
    // input() :state -> unpack_state -> pack_state -> output() :state
    //
    // We want each loop iteration to have its own state container. So we'll
    // insert pack/unpack_state_list_n terms so that each iteration accesses
    // state from a list. The result will look like:
    //
    // input() :state -> unpack_state_list_n(index) -> unpack_state -> pack_state
    // -> pack_state_list_n(index) -> output() :state
    
    // First insert the unpack_state_list_n call
    Term* stateInput = find_state_input(contents);

    // Nothing to do if there's no state input
    if (stateInput == NULL)
        return;

    // Nothing to do if unpack_state_list_n term already exists
    if (find_user_with_function(stateInput, FUNCS.unpack_state_list_n) != NULL)
        return;

    Term* unpackState = find_user_with_function(stateInput, FUNCS.unpack_state);
    ca_assert(unpackState != NULL);

    Term* index = for_loop_find_index(contents);

    Term* unpackStateList = apply(contents, FUNCS.unpack_state_list_n,
        TermList(stateInput, index));
    transfer_users(stateInput, unpackStateList);
    move_before(unpackStateList, unpackState);
    set_input(unpackState, 0, unpackStateList);

    // Now insert the pack_state_list_n call
    Term* stateResult = find_open_state_result(contents, contents->length());

    Term* packStateList = apply(contents, FUNCS.pack_state_list_n,
        TermList(stateInput, stateResult, index));
    packStateList->setBoolProp("final", true);
    move_after(packStateList, stateResult);

    // Make sure the state output uses this result
    Term* stateOutput = append_state_output(contents);
    set_input(stateOutput, 0, packStateList);
}

void start_for_loop(caStack* stack, bool enableLoopOutput)
{
    Frame* frame = top_frame(stack);
    Block* contents = frame->block;

    // Check if top frame actually contains a for-loop (it might be using the #zero block)
    if (!is_for_loop(contents))
        return;

    // Initialize the loop index
    set_int(get_frame_register(frame, for_loop_find_index(contents)), 0);

    if (enableLoopOutput) {
        // Initialize output value
        set_int(get_frame_register(frame, for_loop_find_output_index(contents)), 0);
        caValue* listInput = circa_input(stack, 0);
        set_list(get_frame_register_from_end(frame, 0), list_length(listInput));
    }

    // Interpreter will run the contents of the block
}

void for_loop_finish_iteration(Stack* stack, bool enableLoopOutput)
{
    INCREMENT_STAT(LoopFinishIteration);

    Frame* frame = top_frame(stack);
    Block* contents = frame->block;

    // Find list length
    caValue* listInput = get_frame_register(frame, 0);

    // Increment the loop index
    caValue* index = get_top_register(stack, for_loop_find_index(contents));
    set_int(index, as_int(index) + 1);

    // Preserve list output
    if (enableLoopOutput && frame->exitType != name_Discard) {
        caValue* outputIndex = get_frame_register(frame, for_loop_find_output_index(contents));

        Term* outputPlaceholder = get_output_placeholder(contents, 0);
        caValue* outputList = get_frame_register(frame, outputPlaceholder);
        caValue* outputValue = find_stack_value_for_term(stack, outputPlaceholder->input(0), 0);

        if (!is_list(outputList))
            set_list(outputList);
        list_touch(outputList);
        copy(outputValue, list_get(outputList, as_int(outputIndex)));

        INCREMENT_STAT(LoopWriteOutput);

        // Advance output index
        set_int(outputIndex, as_int(outputIndex) + 1);
    }

    // Check if we are finished
    if (as_int(index) >= list_length(listInput)
            || frame->exitType == name_Break
            || frame->exitType == name_Return) {

        // Possibly truncate output list, in case any elements were discarded.
        if (enableLoopOutput) {
            caValue* outputIndex = get_frame_register(frame, for_loop_find_output_index(contents));
            Term* outputPlaceholder = get_output_placeholder(contents, 0);
            caValue* outputList = get_frame_register(frame, outputPlaceholder);
            list_resize(outputList, as_int(outputIndex));
        } else {
            Term* outputPlaceholder = get_output_placeholder(contents, 0);
            caValue* outputList = get_frame_register(frame, outputPlaceholder);
            set_list(outputList, 0);
        }
        
        finish_frame(stack);
        return;
    }

    // If we're not finished yet, copy rebound outputs back to inputs.
    for (int i=1;; i++) {
        Term* input = get_input_placeholder(contents, i);
        if (input == NULL)
            break;
        Term* output = get_output_placeholder(contents, i);
        copy(get_frame_register(frame, output),
            get_frame_register(frame, input));

        INCREMENT_STAT(Copy_LoopCopyRebound);
    }

    // Return to start of loop body
    frame->pc = 0;
    frame->nextPc = 0;
    frame->exitType = name_None;
}

void finish_while_loop(Term* whileTerm)
{
    Block* block = nested_contents(whileTerm);

    // Append a call to unbounded_loop_finish()
    Term* term = apply(block, FUNCS.unbounded_loop_finish,
        TermList());
    move_before_outputs(term);
}

void evaluate_unbounded_loop(caStack* stack)
{
    Block* contents = (Block*) circa_caller_block(stack);

    // Check for zero evaluations
    if (!as_bool(circa_input(stack, 0))) {
        return;
    }

    push_frame(stack, contents);
}

void evaluate_unbounded_loop_finish(caStack* stack)
{
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
    as_function(index_func)->postCompile = index_func_postCompile;
}

} // namespace circa
