// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "interpreter.h"
#include "function.h"
#include "if_block.h"
#include "importing.h"
#include "inspection.h"
#include "kernel.h"
#include "source_repro.h"
#include "term.h"

namespace circa {

static Term* find_exit_point_for_term(Term* term);
static ExitRank get_exit_level_rank(Symbol level);
static Symbol max_exit_level(Symbol left, Symbol right);
static Symbol get_highest_exit_level(Block* block);

static ExitRank get_exit_level_rank(Symbol level)
{
    if (level == name_Return)
        return EXIT_RANK_SUBROUTINE;
    else if (level == name_Break || level == name_Continue || level == name_Discard)
        return EXIT_RANK_LOOP;
    else
        return EXIT_RANK_NONE;
}

static Symbol max_exit_level(Symbol left, Symbol right)
{
    if (get_exit_level_rank(left) >= get_exit_level_rank(right))
        return left;
    else
        return right;
}

void controlFlow_postCompile(Term* term)
{
    // If this is a return() then give it the special name #return
    if (term->function == FUNCS.return_func) {
        Value return_str;
        set_string(&return_str, "#return");
        rename(term, &return_str);
    }

    // Create a #control value
    Term* controlTerm = create_value(term->owningBlock, TYPES.symbol, "#control");
    hide_from_source(controlTerm);

    Symbol controlValue = name_None;
    if (term->function == FUNCS.return_func)
        controlValue = name_Return;
    else if (term->function == FUNCS.break_func)
        controlValue = name_Break;
    else if (term->function == FUNCS.continue_func)
        controlValue = name_Continue;
    else if (term->function == FUNCS.discard)
        controlValue = name_Discard;

    set_symbol(term_value(controlTerm), controlValue);

    // Add an exit_point after each control-flow term
    Block* block = term->owningBlock;
    Term* exitPoint = apply(block, FUNCS.exit_point, TermList(controlTerm));
    hide_from_source(exitPoint);
}

void break_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "break", term, name_Keyword);
}
void continue_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "continue", term, name_Keyword);
}
void discard_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "discard", term, name_Keyword);
}

void return_formatSource(caValue* source, Term* term)
{
    if (term->boolProp("syntax:returnStatement", false)) {
        append_phrase(source, "return", term, name_Keyword);
        append_phrase(source,
                term->stringProp("syntax:postKeywordWs", " "),
                term, name_Whitespace);

        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            if (inputIndex != 0)
                append_phrase(source, ", ", term, name_None);
            format_source_for_input(source, term, inputIndex, "", "");
        }
    } else {
        format_term_source_default_formatting(source, term);
    }
}

void control_flow_setup_funcs(Block* kernel)
{
    ca_assert(kernel->get("return") == NULL);

    FUNCS.exit_point = import_function(kernel, NULL, "exit_point(any outs :multiple :optional)");

    FUNCS.return_func = import_function(kernel, NULL,
        "return(any outs :multiple :optional) -> any");
    block_set_evaluation_empty(function_contents(FUNCS.return_func), true);
    as_function(FUNCS.return_func)->formatSource = return_formatSource;
    as_function(FUNCS.return_func)->postCompile = controlFlow_postCompile;

    FUNCS.discard = import_function(kernel, NULL, "discard()");
    as_function(FUNCS.discard)->formatSource = discard_formatSource;
    as_function(FUNCS.discard)->postCompile = controlFlow_postCompile;

    FUNCS.break_func = import_function(kernel, NULL, "break()");
    as_function(FUNCS.break_func)->formatSource = break_formatSource;
    as_function(FUNCS.break_func)->postCompile = controlFlow_postCompile;

    FUNCS.continue_func = import_function(kernel, NULL, "continue()");
    as_function(FUNCS.continue_func)->formatSource = continue_formatSource;
    as_function(FUNCS.continue_func)->postCompile = controlFlow_postCompile;
}

static Term* find_exit_point_for_term(Term* term)
{
    // Walk forward and find the nearest exit_point call. If we pass a named term, then
    // we'll have to return NULL, because it means that this term has no appropriate
    // exit_point().
    //
    // One property that we must hold:
    //     Let term A be the result of find_exit_point_for_term(B).
    //     Then, for any term X, find_intermediate_result_for_output(A, X) must equal
    //     find_intermediate_result_for_output(B, X)

    Block* block = term->owningBlock;

    for (int index = term->index + 1; index < block->length(); index++) {
        Term* neighbor = block->get(index);
        if (neighbor == NULL)
            continue;

        if (neighbor->function == FUNCS.exit_point)
            return neighbor;

        // Ignore extra_output and #control terms
        if (neighbor->name == "#control" || neighbor->function == FUNCS.extra_output)
            continue;

        if (neighbor->name != "")
            return NULL;
    }

    return NULL;
}

static Symbol get_highest_exit_level(Block* block)
{
    Symbol highest = name_None;

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);

        if (term->function != FUNCS.exit_point)
            continue;

        highest = max_exit_level(highest, 
            term->intProp("highestExitLevel", name_None));
    }

    return highest;
}

// Returns the highest exit level that can escape out of this term.
Symbol find_highest_escaping_exit_level(Term* term)
{
    // Term's function might be NULL during bootstrapping.
    if (term->function == NULL)
        return name_None;

    // Check if this is just an exiting term.
    if (term->function == FUNCS.return_func)
        return name_Return;
    else if (term->function == FUNCS.break_func)
        return name_Break;
    else if (term->function == FUNCS.continue_func)
        return name_Continue;
    else if (term->function == FUNCS.discard)
        return name_Discard;

    // Check the nested block for this term, and figure out the highest possible
    // exit level.
    Block* block = term->nestedContents;

    if (block == NULL)
        return name_None;

    // Don't look at subroutine declaration contents.
    if (is_major_block(block))
        return name_None;

    Symbol highestLevel = name_None;

    if (term->function == FUNCS.if_block) {
        // For an if-block, we need to iterate over each case block.
        for (int i=0; i < block->length(); i++) {
            Block* caseBlock = block->get(i)->nestedContents;
            if (caseBlock == NULL)
                continue;

            highestLevel = max_exit_level(highestLevel, get_highest_exit_level(caseBlock));
        }
    } else {
        highestLevel = get_highest_exit_level(block);
    }

    // Check if this exit level will actually escape.

    // Exit level 'loop' does not escape the for-loop.
    if (term->function == FUNCS.for_func
            && get_exit_level_rank(highestLevel) == EXIT_RANK_LOOP)
        return name_None;

    // Otherwise, this is the escaping level.
    return highestLevel;
}

void force_term_to_output_to_parent(Term* term)
{
    Block* block = term->owningBlock;
    if (term->name == "")
        return;

    // If this term is inside an if-block, then add it as a block output.
    if (is_case_block(block)) {
        Block* ifBlock = get_block_for_case_block(block);
        Term* existing = if_block_get_output_by_name(ifBlock, term->name.c_str());
        if (existing == NULL) {
            if_block_append_output(ifBlock, &term->nameValue);
        } else {
            // Connect to existing output
            set_input(find_output_placeholder_with_name(block, &term->nameValue),
                0, term);
        }
    } else if (is_minor_block(block)) {

        Term* existing = find_output_placeholder_with_name(block, &term->nameValue);
        if (existing == NULL) {
            Term* placeholder = append_output_placeholder(block, term);
            rename(placeholder, &term->nameValue);
        }
    }
}

void update_exit_points(Block* block)
{
    // Don't insert exit_points in if_block's block (insert in cases instead).
    if (block->owningTerm != NULL && block->owningTerm->function == FUNCS.if_block)
        return;

    for (BlockIteratorFlat it(block); it.unfinished(); it.advance()) {
        Term* term = it.current();

        if (term->name == "#return" && !is_output_placeholder(term)) {
            for (int outputIndex=0; outputIndex < term->numInputs(); outputIndex++) {
                Term* returnValue = term->input(outputIndex);
                if (returnValue == NULL)
                    continue;

                force_term_to_output_to_parent(returnValue);

                // If this is a subroutine, make sure that the primary output is properly
                // connected.
                if (is_major_block(block)) {
                    Term* output = get_output_placeholder(block, outputIndex);
                    if (output != NULL) {
                        set_input(output, 0, returnValue);
                    }
                }
            }
        }

        Symbol escapingExitLevel = find_highest_escaping_exit_level(term);
        if (get_exit_level_rank(escapingExitLevel) > EXIT_RANK_NONE) {

            // This term can cause this block to exit.

            // Make sure that there is an exit_point call that follows this term.
            Term* exitPoint = find_exit_point_for_term(term);

            Term* controlVar = NULL;

            if (exitPoint == NULL) {
                // Create a new exit_point()
                exitPoint = apply(block, FUNCS.exit_point, TermList(NULL));
                move_after(exitPoint, term);

                // Make sure we find a #control term that is in term's extra outputs.
                controlVar = find_name_at(exitPoint, "#control");
                set_input(exitPoint, 0, controlVar);
            } else {
                controlVar = find_name_at(exitPoint, "#control");
            }

            exitPoint->setIntProp("highestExitLevel", escapingExitLevel);

            // If this exit_point is inside an if-block, then we should add #control as
            // a block output.
            if (controlVar != NULL)
                force_term_to_output_to_parent(controlVar);
        }
    }

    // 2nd pass, now that we have finished creating derived terms, go back
    // and ensure that the inputs to each exit_point are corrent.
    for (BlockIteratorFlat it(block); it.unfinished(); it.advance()) {

        Term* exitPoint = it.current();

        if (exitPoint->function != FUNCS.exit_point)
            continue;

        // For each output in this block, assign an input to the exit_point that
        // has the 'intermediate' result at this term's location. (such as, if the output
        // has a name, use the term at this location with the given name.

        for (int i=0;; i++) {
            Term* output = get_output_placeholder(block, i);
            if (output == NULL)
                break;

            Term* intermediate = find_intermediate_result_for_output(exitPoint, output);

            // exit_point() uses input 0 for control flow, so assign each input to i+1.
            set_input(exitPoint, i + 1, intermediate);
        }
    }
}

} // namespace circa
