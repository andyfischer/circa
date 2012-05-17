// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "evaluation.h"
#include "function.h"
#include "if_block.h"
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

    caValue* args = circa_input(stack, 0);

    caValue* control = circa_index(args, 0);

    // Only exit if the control says we should exit
    if (is_null(control))
        return;

    int intermediateOutputCount = circa_count(args) - 1;

    // Copy intermediate values to the frame's output placeholders.
    for (int i=0; i < intermediateOutputCount; i++) {
        caValue* result = circa_index(args, i + 1);
        caValue* out = get_frame_register_from_end(frame, i);
        if (result != NULL)
            copy(result, out);
        else
            set_null(out);
    }

    // Set PC to end
    frame->nextPc = frame->endPc;
    frame->exitType = as_name(control);
}

void evaluate_return(caStack* stack)
{
    copy(circa_input(stack, 0), circa_output(stack, 0));
}

/*
Name get_interrupt_level(Term* term)
{
    if (term->function == FUNCS.return_func)
        return name_Return;
    else if (term->function == FUNCS.break_func)
        return name_Break;
    else if (term->function == FUNCS.break_func)
        return name_Continue;
    else if (term->function == FUNCS.break_func)
        return name_Return;

    return name_None;
}
*/

void controlFlow_postCompile(Term* term)
{
    // If this is a return() then give it the special name #return
    if (term->function == FUNCS.return_func)
        rename(term, "#return");

    // Create a #control value
    Term* controlTerm = create_value(term->owningBranch, &NAME_T, "#control");
    hide_from_source(controlTerm);

    Name controlValue = name_None;
    if (term->function == FUNCS.return_func)
        controlValue = name_Return;
    else if (term->function == FUNCS.break_func)
        controlValue = name_Break;
    else if (term->function == FUNCS.continue_func)
        controlValue = name_Continue;
    else if (term->function == FUNCS.discard)
        controlValue = name_Discard;

    set_name(term_value(controlTerm), controlValue);

    // Add an exit_point after each control-flow term
    Branch* branch = term->owningBranch;
    Term* exitPoint = apply(branch, FUNCS.exit_point, TermList(controlTerm));
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
        "return(any :optional) -> any");
    as_function(FUNCS.return_func)->formatSource = return_formatSource;
    as_function(FUNCS.return_func)->postCompile = controlFlow_postCompile;

    FUNCS.discard = import_function(kernel, NULL, "discard()");
    as_function(FUNCS.discard)->formatSource = discard_formatSource;
    as_function(FUNCS.discard)->postCompile = controlFlow_postCompile;
    hide_from_docs(FUNCS.discard);

    FUNCS.break_func = import_function(kernel, NULL, "break()");
    as_function(FUNCS.break_func)->formatSource = break_formatSource;
    as_function(FUNCS.break_func)->postCompile = controlFlow_postCompile;
    hide_from_docs(FUNCS.break_func);

    FUNCS.continue_func = import_function(kernel, NULL, "continue()");
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
    //     Let term A be the result of find_exit_point_for_term(B).
    //     Then, for any term X, find_intermediate_result_for_output(A, X) must equal
    //     find_intermediate_result_for_output(B, X)

    Branch* branch = term->owningBranch;

    for (int index = term->index + 1; index < branch->length(); index++) {
        Term* neighbor = branch->get(index);
        if (neighbor == NULL)
            continue;

        if (neighbor->function == FUNCS.exit_point)
            return neighbor;

        // Ignore extra_output and #control terms
        if (neighbor->name == "#control" || neighbor->function == EXTRA_OUTPUT_FUNC)
            continue;

        if (neighbor->name != "")
            return NULL;
    }

    return NULL;
}

// Returns whether the given term can interrupt the control flow of this
// branch.
bool can_interrupt_control_flow(Term* term, Branch* branch)
{
    // An exiting term can always interrupt the branch it is in.
    if (term->function == FUNCS.return_func
            || term->function == FUNCS.continue_func
            || term->function == FUNCS.break_func
            || term->function == FUNCS.discard)
        return true;

    // Check nested minor branches.
    if (term->nestedContents != NULL && is_minor_branch(term->nestedContents)) {
        Branch* nestedBranch = term->nestedContents;
        for (int i=0; i < nestedBranch->length(); i++) {
            // Recurse looking for a nested exit call.
            //
            // This could be made more efficient by not recursing - it only needs
            // to look 1 or 2 levels deep for an exit_point call. (it's 2 levels
            // deep for an if-block).

            if (can_interrupt_control_flow(nestedBranch->get(i), branch))
                return true;
        }
    }

    return false;
}

void force_term_to_output_to_parent(Term* term)
{
    Branch* branch = term->owningBranch;
    ca_assert(term->name != "");

    // If this term is inside an if-block, then add it as a block output.
    if (is_case_branch(branch)) {
        Branch* ifBlock = get_block_for_case_branch(branch);
        Term* existing = if_block_get_output_by_name(ifBlock, term->name.c_str());
        if (existing == NULL) {
            if_block_append_output(ifBlock, term->name.c_str());
        } else {
            // Connect to existing output
            set_input(find_output_placeholder_with_name(branch, term->name.c_str()),
                0, term);
        }
    } else if (is_minor_branch(branch)) {

        Term* existing = find_output_placeholder_with_name(branch, term->name.c_str());
        if (existing == NULL) {
            Term* placeholder = append_output_placeholder(branch, term);
            rename(placeholder, term->name.c_str());
        }
    }
}

void update_exit_points(Branch* branch)
{
    // Don't insert exit_points inside an if_block
    if (branch->owningTerm != NULL && branch->owningTerm->function == FUNCS.if_block)
        return;

    for (BranchIteratorFlat it(branch); it.unfinished(); it.advance()) {
        Term* term = it.current();

        if (term->name == "#return" && !is_output_placeholder(term)) {
            force_term_to_output_to_parent(term);

            // If this is a subroutine, make sure that the primary output is properly connected.
            if (is_major_branch(branch)) {
                Term* output = get_output_placeholder(branch, 0);
                if (output != NULL) {
                    set_input(output, 0, term);
                }
            }
        }

        if (can_interrupt_control_flow(term, branch)) {

            // Make sure that there is an exit_point call that follows this term.
            Term* exitPoint = find_exit_point_for_term(term);

            Term* controlVar = NULL;

            if (exitPoint == NULL) {
                // Create a new exit_point()
                controlVar = find_name(branch, "#control");
                exitPoint = apply(branch, FUNCS.exit_point, TermList(controlVar));
                move_after(exitPoint, term);
            } else {
                controlVar = find_name(branch, "#control");
            }

            // If this exit_point is inside an if-block, then we should add #control as
            // a block output.
            if (controlVar != NULL)
                force_term_to_output_to_parent(controlVar);

            // For each output in this branch, assign an input to the exit_point that
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
