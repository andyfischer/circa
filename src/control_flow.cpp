// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "evaluation.h"
#include "function.h"
#include "importing.h"
#include "introspection.h"
#include "documentation.h"
#include "kernel.h"
#include "source_repro.h"
#include "term.h"

namespace circa {

void evaluate_exit_point(caStack* stack)
{
    Frame* frame = get_frame(stack, 1);
    ca_assert(frame != NULL);

    // TODO: check input 0

    int inputCount = circa_num_inputs(stack);
    int intermediateOutputCount = inputCount - 1;

    // Copy intermediate values to the frame's output placeholders.
    for (int i=0; i < intermediateOutputCount; i++) {
        caValue* result = circa_input(stack, i + 1);
        caValue* out = get_frame_register_from_end(frame, i);
        if (result != NULL)
            copy(result, out);
        else
            set_null(out);
    }

    // Set PC to end
    frame->nextPc = frame->endPc;
}

void evaluate_return(caStack* stack)
{
    set_name(circa_output(stack, 0), name_Return);
#if 0
    // Grab the output value
    circa::Value value;
    swap(circa_input(stack, 0), &value);

    // Discard caller branch
    pop_frame(stack);

    // Mark above frames as finished, until we find the enclosing subroutine.
    for (int i=1;; i++) {
        Frame* frame = top_frame(stack);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (is_subroutine(frame->branch->owningTerm)) {

            // Copy our output
            copy(&value, get_frame_register_from_end(frame, 0));

            // Done
            break;
        }

        finish_frame(stack);
    }
#endif
}

void evaluate_break(caStack* stack)
{
    set_name(circa_output(stack, 0), name_Break);
#if 0
    // Discard caller branch
    pop_frame(stack);

    // Mark above frames as finished, until we find the enclosing for loop.
    while (true) {
        Frame* frame = top_frame(stack);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func)
            break;

        finish_frame(stack);
    }
#endif
}
void evaluate_continue(caStack* stack)
{
    set_name(circa_output(stack, 0), name_Continue);
#if 0
    // Discard caller branch
    pop_frame(stack);

    // Mark above frames as finished, until we find the enclosing for loop.
    while (true) {
        Frame* frame = top_frame(stack);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func)
            break;

        finish_frame(stack);
    }
#endif
}
void evaluate_discard(caStack* stack)
{
    set_name(circa_output(stack, 0), name_Discard);
#if 0
    // Discard caller branch
    pop_frame(stack);

    // Mark above frames as finished, until we find the enclosing for loop.
    while (true) {
        Frame* frame = top_frame(stack);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func) {
            frame->discarded = true;
            break;
        }

        finish_frame(stack);
    }
#endif
}

void controlFlow_postCompile(Term* term)
{
    // Give the name "#control" to the term
    rename(term, "#control");

    // Add an exit_point after each control-flow term
    Branch* branch = term->owningBranch;
    Term* exitPoint = apply(branch, FUNCS.exit_point, TermList(term));
    hide_from_source(exitPoint);
}

void break_formatSource(StyledSource* source, Term* term)
{
    append_phrase(source, "break", term, phrase_type::KEYWORD);
}
void continue_formatSource(StyledSource* source, Term* term)
{
    append_phrase(source, "continue", term, phrase_type::KEYWORD);
}
void discard_formatSource(StyledSource* source, Term* term)
{
    append_phrase(source, "discard", term, phrase_type::KEYWORD);
}


void return_formatSource(StyledSource* source, Term* term)
{
    if (term->boolPropOptional("syntax:returnStatement", false)) {
        append_phrase(source, "return", term, phrase_type::KEYWORD);
        append_phrase(source,
                term->stringPropOptional("syntax:postKeywordWs", " "),
                term, phrase_type::WHITESPACE);

        if (term->input(0) != NULL)
            format_source_for_input(source, term, 0, "", "");
    } else {
        format_term_source_default_formatting(source, term);
    }
}

void control_flow_setup_funcs(Branch* kernel)
{
    ca_assert(kernel->get("return") == NULL);

    FUNCS.exit_point = import_function(kernel, evaluate_exit_point,
        "exit_point(any :multiple :optional)");

    FUNCS.return_func = import_function(kernel, evaluate_return,
        "return(any :optional) -> Name");
    as_function(FUNCS.return_func)->formatSource = return_formatSource;
    as_function(FUNCS.return_func)->postCompile = controlFlow_postCompile;

    FUNCS.discard = import_function(kernel, evaluate_discard,
        "discard() -> Name");
    as_function(FUNCS.discard)->formatSource = discard_formatSource;
    as_function(FUNCS.discard)->postCompile = controlFlow_postCompile;
    hide_from_docs(FUNCS.discard);

    FUNCS.break_func = import_function(kernel, evaluate_break,
        "break() -> Name");
    as_function(FUNCS.break_func)->formatSource = break_formatSource;
    as_function(FUNCS.break_func)->postCompile = controlFlow_postCompile;
    hide_from_docs(FUNCS.break_func);

    FUNCS.continue_func = import_function(kernel, evaluate_continue,
        "continue() -> Name");
    as_function(FUNCS.continue_func)->formatSource = continue_formatSource;
    as_function(FUNCS.continue_func)->postCompile = controlFlow_postCompile;
    hide_from_docs(FUNCS.continue_func);
}

void early_finish_frame(caStack* stack, Frame* frame)
{
    Branch* branch = frame->branch;

    // Find the next exit_point() call
    Term* exitPoint = NULL;
    for (int i = frame->pc; i < branch->length(); i++) {
        if (branch->get(i)->function == FUNCS.exit_point) {
            exitPoint = branch->get(i);
            break;
        }
    }

    if (exitPoint != NULL) {
        // Copy values to this branch's output placeholders.
        for (int i=0; i < exitPoint->numInputs(); i++) {
            Term* resultTerm = exitPoint->input(i);
            caValue* result = find_stack_value_for_term(stack, resultTerm, 0);
            caValue* out = get_frame_register_from_end(frame, i);
            if (result != NULL)
                copy(result, out);
            else
                set_null(out);
        }
    }

    // Set PC to end
    frame->nextPc = frame->endPc;
}

Term* find_input_placeholder_with_name(Branch* branch, const char* name)
{
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(branch, i);
        if (placeholder == NULL)
            return NULL;
        if (placeholder->name == name)
            return placeholder;
    }
}

Term* find_exit_point_for_term(Term* term)
{
    // Walk forward and find the nearest exit_point call. If we pass a named term, then
    // we'll have to return NULL, because it means that this term has no appropriate
    // exit_point().
    //
    // One property that we must hold:
    //     Say that term A is the result of find_exit_point_for_term(B).
    //     Then, for any term X, find_intermediate_result_for_output(A, X) must equal
    //     find_intermediate_result_for_output(B, X)

    Branch* branch = term->owningBranch;

    for (int index = term->index + 1; index < branch->length(); index++) {
        Term* neighbor = branch->get(index);
        if (neighbor == NULL)
            continue;

        if (neighbor->function == FUNCS.exit_point)
            return neighbor;

        if (neighbor->name != "")
            return NULL;
    }

    return NULL;
}

bool can_interrupt_control_flow(Term* term)
{
    if (term->function == FUNCS.return_func
            || term->function == FUNCS.continue_func
            || term->function == FUNCS.break_func
            || term->function == FUNCS.discard)
        return true;

    // Check nested minor branches.
    if (term->nestedContents != NULL && is_minor_branch(term->nestedContents)) {
        Branch* branch = term->nestedContents;
        for (int i=0; i < branch->length(); i++) {
            // Recurse looking for a nested exit call.
            //
            // This could be made more efficient by not recursing - it only needs
            // to look 1 or 2 levels deep for an exit_point call. (it's 2 levels
            // deep for an if-block).

            if (can_interrupt_control_flow(branch->get(i)))
                return true;
        }
    }

    return false;
}

void update_exit_points(Branch* branch)
{
    // Don't insert exit_points inside an if_block
    if (branch->owningTerm != NULL && branch->owningTerm->function == FUNCS.if_block)
        return;

    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = it.current();

        if (can_interrupt_control_flow(term)) {

            // Make sure that there is an exit_point call that follows this term.
            Term* exitPoint = find_exit_point_for_term(term);

            if (exitPoint == NULL) {
                // Create a new exit_point()
                exitPoint = apply(branch, FUNCS.exit_point, TermList());
                move_after(exitPoint, term);
            }

            // For each output in this branch, assign an input to the given term that
            // has the 'intermediate' result at this term's location. (such as, if the output
            // has a name, use the term at this location with the given name.

            for (int i=0;; i++) {
                Term* output = get_output_placeholder(branch, i);
                if (output == NULL)
                    break;

                Term* intermediate = find_intermediate_result_for_output(exitPoint, output);
                // exit_point() uses input 0 for control flow, so assign each input to i+1.
                set_input(exitPoint, i + 1, intermediate);
            }
        }
    }
}

} // namespace circa
