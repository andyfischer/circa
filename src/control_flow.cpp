// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "evaluation.h"
#include "function.h"
#include "importing.h"
#include "documentation.h"
#include "kernel.h"
#include "source_repro.h"
#include "term.h"

namespace circa {

void evaluate_return(caStack* stack)
{
    // Grab the output value
    circa::Value value;
    swap(circa_input(stack, 0), &value);

    // Mark above frames as finished, until we find the enclosing subroutine.
    for (int i=1;; i++) {
        Frame* frame = get_frame(stack, i);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (is_subroutine(frame->branch->owningTerm)) {

            // Copy our output
            copy(&value, get_frame_register_from_end(frame, 0));

            // Done
            break;
        }
    }
}

void evaluate_break(caStack* stack)
{
    // Mark above frames as finished, until we find the enclosing for loop.
    for (int i=1;; i++) {
        Frame* frame = get_frame(stack, i);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func)
            break;
    }

}
void evaluate_continue(caStack* stack)
{
    // Mark above frames as finished, until we find the enclosing for loop.
    for (int i=1;; i++) {
        Frame* frame = get_frame(stack, i);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func)
            break;
    }
}
void evaluate_discard(caStack* stack)
{
    // Mark above frames as finished, until we find the enclosing for loop.
    for (int i=1;; i++) {
        Frame* frame = get_frame(stack, i);
        ca_assert(frame != NULL);

        early_finish_frame(stack, frame);

        if (frame->branch->owningTerm->function == FUNCS.for_func) {
            frame->discarded = true;
            break;
        }
    }
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

    FUNCS.exit_point = import_function(kernel, NULL, "exit_point(any :multiple :optional)");

    FUNCS.return_func = import_function(kernel, evaluate_return, "return(any :optional)");
    as_function(FUNCS.return_func)->formatSource = return_formatSource;

    FUNCS.discard = import_function(kernel, evaluate_discard, "discard()");
    as_function(FUNCS.discard)->formatSource = discard_formatSource;
    hide_from_docs(FUNCS.discard);

    FUNCS.break_func = import_function(kernel, evaluate_break, "break()");
    as_function(FUNCS.break_func)->formatSource = break_formatSource;
    hide_from_docs(FUNCS.break_func);

    FUNCS.continue_func = import_function(kernel, evaluate_continue, "continue()");
    as_function(FUNCS.continue_func)->formatSource = continue_formatSource;
    hide_from_docs(FUNCS.continue_func);
}

void early_finish_frame(caStack* stack, Frame* frame)
{
    Branch* branch = frame->branch;

    // Find the preceeding exit_point() call
    Term* exitPoint = NULL;
    for (int i = frame->pc - 1; i >= 0; i--)
        if (branch->get(i)->function == FUNCS.exit_point) {
            exitPoint = branch->get(i);
            break;
        }

    ca_assert(exitPoint != NULL);

    // Copy values to this branch's output placeholders.
    for (int i=0; i < exitPoint->numInputs(); i++) {
        caValue* result = find_stack_value_for_term(stack, exitPoint->input(i), 0);
        caValue* out = get_frame_register_from_end(frame, i);
        if (result != NULL)
            copy(result, out);
        else
            set_null(out);
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
    // Walk backwards and find the nearest exit_point call. If we pass a named term, then
    // we'll have to return NULL, because it means that this term has no appropriate
    // exit_point().
    //
    // One property that we must hold:
    //     Say that term A is the result of find_exit_point_for_term(B).
    //     Then, for any term X, find_intermediate_result_for_output(A, X) must equal
    //     find_intermediate_result_for_output(B, X)

    Branch* branch = term->owningBranch;

    for (int index = term->index - 1; index >= 0; index--) {
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
    if (term->function == FUNCS.if_block
            || term->function == FUNCS.for_func
            || term->function == FUNCS.case_func) {
        Branch* branch = term->nestedContents;
        for (int i=0; i < branch->length(); i++) {
            if (can_interrupt_control_flow(branch->get(i)))
                return true;
        }
    }

    return false;
}

void update_exit_points(Branch* branch)
{
    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = it.current();

        if (can_interrupt_control_flow(term)) {

            // Make sure that there is an exit_point call that preceeds this term.
            Term* exitPoint = find_exit_point_for_term(term);

            if (exitPoint == NULL) {
                // Create a new exit_point()
                exitPoint = apply(branch, FUNCS.exit_point, TermList());
                move_before(exitPoint, term);
            }

            // For each output in this branch, assign an input to the given term that
            // has the 'intermediate' result at this term's location. (such as, if the output
            // has a name, use the term at this location with the given name.

            for (int i=0;; i++) {
                Term* output = get_output_placeholder(branch, i);
                if (output == NULL)
                    break;

                Term* intermediate = find_intermediate_result_for_output(exitPoint, output);
                set_input(exitPoint, i, intermediate);
            }
        }
    }
}

} // namespace circa
