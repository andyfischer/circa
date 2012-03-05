// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "importing_macros.h"
#include "introspection.h"
#include "list.h"
#include "locals.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"

#include "loops.h"

namespace circa {

void for_loop_fix_state_input(Branch* contents);

Term* for_loop_get_iterator(Branch* contents)
{
    for (int i=0; i < contents->length(); i++)
        if (contents->get(i)->function == FUNCS.get_index)
            return contents->get(i);
    return NULL;
}

Term* for_loop_find_index(Branch* contents)
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

Branch* get_for_loop_outer_rebinds(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    return contents->getFromEnd(0)->contents();
}

Term* start_building_for_loop(Term* forTerm, const char* iteratorName)
{
    Branch* contents = nested_contents(forTerm);

    // Add input placeholder for the list input
    Term* listInput = apply(contents, FUNCS.input, TermList());

    // Add loop_index()
    Term* index = apply(contents, FUNCS.loop_index, TermList(listInput));
    hide_from_source(index);

    // Add loop_iterator()
    Term* iterator = apply(contents, FUNCS.get_index, TermList(listInput, index),
        iteratorName);
    change_declared_type(iterator, infer_type_of_get_index(forTerm->input(0)));
    hide_from_source(iterator);
    return iterator;
}

void add_loop_output_term(Branch* branch)
{
    Term* result = NULL;

    // Find the term to use for the for-loop output. If the loop iterater name
    // was rebound then use that.
    Term* iterator = for_loop_get_iterator(branch);

    Term* lastWithIteratorName = branch->get(iterator->name);

    // For a rebound list, always use the last-with-iterator-name, even if it's
    // the iterator itself.
    if (branch->owningTerm->boolPropOptional("modifyList", false))
        result = lastWithIteratorName;
    
    if (result == NULL && lastWithIteratorName != iterator)
        result = lastWithIteratorName;

    // Otherwise, use the last expression as the output.
    if (result == NULL)
        result = find_last_non_comment_expression(branch);

    Term* term = apply(branch, FUNCS.loop_output,
        TermList(for_loop_find_index(branch), result));
    move_before_outputs(term);
}

void add_implicit_placeholders(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);
    std::string listName = forTerm->input(0)->name;
    Term* iterator = for_loop_get_iterator(contents);
    std::string iteratorName = iterator->name;

    std::vector<std::string> reboundNames;
    list_names_that_this_branch_rebinds(contents, reboundNames);

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

        Term* input = apply(contents, FUNCS.input, TermList(), name);
        change_declared_type(input, original->type);
        contents->move(input, inputIndex);

        set_input(forTerm, inputIndex, original);

        // Repoint terms to use our new input_placeholder
        for (int i=0; i < contents->length(); i++)
            remap_pointers_quick(contents->get(i), original, input);

        Term* term = apply(contents, FUNCS.output, TermList(result), name);

        // Move output into the correct output slot
        contents->move(term, contents->length() - 1 - inputIndex);

        inputIndex++;
    }
}

void repoint_terms_to_use_input_placeholders(Branch* contents)
{
    // Visit every term
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);

        // Visit every input
        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            Term* input = term->input(inputIndex);
            if (input == NULL)
                continue;
            
            // If the input is outside this branch, then see if we have a named
            // input that could be used instead.
            if (input->owningBranch == contents || input->name == "")
                continue;

            Term* replacement = find_input_placeholder_with_name(contents, input->name.c_str());
            if (replacement == NULL)
                continue;

            set_input(term, inputIndex, replacement);
        }
    }
}

void finish_for_loop(Term* forTerm)
{
    Branch* contents = nested_contents(forTerm);

    // Add a primary output
    apply(contents, FUNCS.output, TermList(NULL));

    pack_any_open_state_vars(contents);
    for_loop_fix_state_input(contents);
    check_to_add_state_output_placeholder(contents);
    add_loop_output_term(contents);

    add_implicit_placeholders(forTerm);
    repoint_terms_to_use_input_placeholders(contents);

    check_to_insert_implicit_inputs(forTerm);
    update_extra_outputs(forTerm);

    set_branch_in_progress(contents, false);
}

Term* find_enclosing_for_loop(Term* term)
{
    if (term == NULL)
        return NULL;

    if (term->function == FOR_FUNC)
        return term;

    Branch* branch = term->owningBranch;
    if (branch == NULL)
        return NULL;

    return find_enclosing_for_loop(branch->owningTerm);
}
Branch* find_enclosing_for_loop_contents(Term* term)
{
    Term* loop = find_enclosing_for_loop(term);
    if (loop == NULL)
        return NULL;
    return nested_contents(loop);
}

void for_loop_fix_state_input(Branch* contents)
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
    move_after(packStateList, stateResult);
}

CA_FUNCTION(evaluate_for_loop)
{
    Term* caller = CALLER;
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(caller);

    caValue* inputList = INPUT(0);
    int inputListLength = inputList->numElements();

    List registers;
    registers.resize(get_locals_count(contents));

    // Copy inputs (first time)
    for (int i=0;; i++) {
        if (get_input_placeholder(contents, i) != NULL)
            copy(INPUT(i), registers[i]);
        else
            break;
    }

    // Create a stack frame
    push_frame(context, contents, &registers);

    // Set up a blank list for output
    set_list(top_frame(context)->registers[contents->length()-1], 0);

    // For a zero-iteration loop, just copy over inputs to their respective outputs.
    if (inputListLength == 0) {
        List* registers = &top_frame(context)->registers;
        for (int i=1;; i++) {
            Term* input = get_input_placeholder(contents, i);
            if (input == NULL)
                break;
            Term* output = get_output_placeholder(contents, i);
            if (output == NULL)
                break;
            copy(registers->get(input->index), registers->get(output->index));
        }
        finish_frame(context);
        return;
    }

    // Walk forward until we find the loop_index() term.
    int loopIndexPos = 0;
    for (; loopIndexPos < contents->length(); loopIndexPos++) {
        if (contents->get(loopIndexPos)->function == FUNCS.loop_index)
            break;
    }
    ca_assert(contents->get(loopIndexPos)->function == FUNCS.loop_index);

    // Set the loop index
    set_int(top_frame(context)->registers[loopIndexPos], 0);

    // Interpreter will run the contents of the branch
}

void for_loop_finish_iteration(EvalContext* context)
{
    Frame* frame = top_frame(context);
    Branch* contents = frame->branch;

    // Hack: make sure the output_placeholder terms have their values
    for (int i=1;; i++) {
        Term* output = get_output_placeholder(contents, i);
        if (output == NULL)
            break;
        copy(get_input(context, output->input(0)), get_input(context, output));
    }

    // Find list length
    caValue* listInput = frame->registers[0];

    // Increment the loop index
    caValue* index = frame->registers[for_loop_find_index(contents)->index];
    set_int(index, as_int(index) + 1);

    // Check if we are finished
    if (as_int(index) >= list_length(listInput)) {
        finish_frame(context);
        return;
    }

    // If we're not finished yet, copy rebound outputs back to inputs.
    for (int i=1;; i++) {
        List* registers = &frame->registers;
        Term* input = get_input_placeholder(contents, i);
        if (input == NULL)
            break;
        Term* output = get_output_placeholder(contents, i);
        copy(registers->get(output->index), registers->get(input->index));
    }

    // Return to start of loop body
    frame->nextPc = 0;
}

CA_FUNCTION(evaluate_loop_output)
{
    Term* caller = CALLER;
    Branch* contents = caller->owningBranch;
    Frame* topFrame = top_frame(CONTEXT);

    caValue* result = INPUT(1);
    
    // Copy loop output
    Term* primaryOutput = get_output_placeholder(contents, 0);
    caValue* output = topFrame->registers[primaryOutput->index];
    copy(result, list_append(output));

    for_loop_finish_iteration(CONTEXT);
}

void finish_while_loop(Term* whileTerm)
{
    Branch* branch = nested_contents(whileTerm);

    // Append a call to unbounded_loop_finish()
    Term* term = apply(branch, FUNCS.unbounded_loop_finish,
        TermList());
    move_before_outputs(term);
}

CA_FUNCTION(evaluate_unbounded_loop)
{
    EvalContext* context = CONTEXT;
    Branch* contents = nested_contents(CALLER);

    // Check for zero evaluations
    if (!as_bool(INPUT(0))) {
        return;
    }

    push_frame(context, contents);
}

CA_FUNCTION(evaluate_unbounded_loop_finish)
{
}

} // namespace circa
