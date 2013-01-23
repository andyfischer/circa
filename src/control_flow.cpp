// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "inspection.h"
#include "interpreter.h"
#include "function.h"
#include "if_block.h"
#include "importing.h"
#include "inspection.h"
#include "kernel.h"
#include "source_repro.h"
#include "string_type.h"
#include "term.h"

namespace circa {

static Symbol max_exit_level(Symbol left, Symbol right);
static void update_inputs_for_exit_point(Term* exitCall, Term* exitPoint);
static void create_implicit_outputs_for_exit_point(Term* exitCall, Term* exitPoint);

bool is_exit_point(Term* term)
{
    return term->function == FUNCS.exit_point;
}

bool is_exit_producer(Term* term)
{
    return term->function == FUNCS.return_func
        || term->function == FUNCS.break_func
        || term->function == FUNCS.continue_func
        || term->function == FUNCS.discard;
}

Symbol term_get_highest_exit_level(Term* term)
{
    if (term->function == FUNCS.return_func)
        return sym_ExitLevelFunction;
    else if (term->function == FUNCS.break_func
            || term->function == FUNCS.continue_func
            || term->function == FUNCS.discard)
        return sym_ExitLevelLoop;
    else if (term->function == FUNCS.exit_point)
        return term_get_int_prop(term, sym_HighestExitLevel, sym_None);
    else
        return sym_None;
}

static Symbol max_exit_level(Symbol left, Symbol right)
{
    if (left == sym_ExitLevelFunction || right == sym_ExitLevelFunction)
        return sym_ExitLevelFunction;
    if (left == sym_ExitLevelLoop || right == sym_ExitLevelLoop)
        return sym_ExitLevelLoop;
    return sym_None;
}

void break_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "break", term, sym_Keyword);
}
void continue_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "continue", term, sym_Keyword);
}
void discard_formatSource(caValue* source, Term* term)
{
    append_phrase(source, "discard", term, sym_Keyword);
}

void return_formatSource(caValue* source, Term* term)
{
    if (term->boolProp("syntax:returnStatement", false)) {
        append_phrase(source, "return", term, sym_Keyword);
        append_phrase(source,
                term->stringProp("syntax:postKeywordWs", " "),
                term, sym_Whitespace);

        for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
            if (inputIndex != 0)
                append_phrase(source, ", ", term, sym_None);
            format_source_for_input(source, term, inputIndex, "", "");
        }
    } else {
        format_term_source_default_formatting(source, term);
    }
}

Term* find_output_placeholder_for_intermediate_term(Term* term)
{
    // TODO
    return NULL;
}

void create_output_from_minor_block(Block* block, caValue* description)
{
    if (is_case_block(block)) {
        Block* ifBlock = get_block_for_case_block(block);
        if_block_append_output(ifBlock, description);
    } else if (is_minor_block(block)) {
        append_output_placeholder_with_description(block, description);
    }
}

void create_block_output_for_term(Term* term)
{
#if 0
    Block* block = term->owningBlock;

    // If this term is inside an if-block, then add it as a block output.
    if (is_case_block(block)) {
        Block* ifBlock = get_block_for_case_block(block);
        Term* existing = if_block_get_output_by_name(ifBlock, term->name.c_str());
        if (existing == NULL) {
            if_block_append_output(ifBlock, term->name.c_str());
        } else {
            // Connect to existing output
            set_input(find_output_placeholder_with_name(block, term->name.c_str()), 0, term);
        }
    } else if (is_minor_block(block)) {

        Term* existing = find_output_placeholder_with_name(block, term->name.c_str());
        if (existing == NULL) {
            Term* placeholder = append_output_placeholder(block, term);
            rename(placeholder, term->nameSymbol);
        }
    }
#endif
}

Symbol term_get_escaping_exit_level(Term* term)
{
    // Check for a call that directly causes exit: return/continue/etc.
    Symbol directExitLevel = term_get_highest_exit_level(term);
    if (directExitLevel != sym_None)
        return directExitLevel;

    // For-block
    if (term->function == FUNCS.for_func) {
        Block* contents = nested_contents(term);

        for (int i=0; i < contents->length(); i++) {
            Symbol exit = term_get_highest_exit_level(contents->get(i));

            // Only ExitRankFunction escapes from for-block.
            if (exit == sym_ExitLevelFunction)
                return exit;
        }

        return sym_None;
    }

    // If-block
    if (term->function == FUNCS.if_block) {
        Block* topContents = nested_contents(term);
        Symbol highestExit = sym_None;
        for (int caseIndex=0; caseIndex < topContents->length(); caseIndex++) {
            Block* caseBlock = nested_contents(topContents->get(caseIndex));
            for (int i=0; i < caseBlock->length(); i++) {

                Symbol exit = term_get_highest_exit_level(caseBlock->get(i));

                // All exits escape from if-block.
                highestExit = max_exit_level(exit, highestExit);
            }
        }
        return highestExit;
    }

    return sym_None;
}

Term* find_trailing_exit_point(Term* term)
{
    Term* lookahead = term;
    while (true) {
        lookahead = following_term(lookahead);
        if (lookahead == NULL)
            return NULL;

        if (lookahead->function == FUNCS.exit_point)
            return lookahead;

        if (lookahead->function != FUNCS.extra_output)
            return NULL;
    }
}

void update_for_control_flow(Block* block)
{
    // Only worry about minor blocks.
    if (!is_minor_block(block))
        return;

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        Symbol escapingExit = term_get_escaping_exit_level(term);

        if (escapingExit != sym_None) {
            // This term can exit, make sure that all necessary values are output to parent branch.
            // TODO
            
            break;
        }
    }

#if 0
    here's the 1/7/13 version

    bool needToRemoveNulls = false;

    // Primary pass: insert or delete exit_point() calls.
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        // Don't examine existing exit_point() calls (examine their owner instead).
        if (term->function == FUNCS.exit_point)
            continue;

        Symbol highestExitLevel = term_get_escaping_exit_level(term);
        Term* existingExitPoint = find_trailing_exit_point(term);

        bool shouldHaveExitPoint = highestExitLevel != sym_None;
        bool hasExitPoint = existingExitPoint != NULL;

        if (shouldHaveExitPoint && !hasExitPoint) {

            // Make sure that a minor block outputs the :control value. Has
            // no effect on major blocks.
            Value controlOutputDesc;
            set_list(&controlOutputDesc, 1);
            set_symbol(list_get(&controlOutputDesc, 0), sym_Control);
            create_output_from_minor_block(block, &controlOutputDesc);

            // Need to add an exit_point().
            Term* exitPoint = apply(block, FUNCS.exit_point, TermList());
            update_inputs_for_exit_point(term, exitPoint);
            term_set_int_prop(exitPoint, sym_HighestExitLevel, highestExitLevel);
            move_after(exitPoint, term);

        } else if (!shouldHaveExitPoint && hasExitPoint) {

            // Need to remove this exit_point.
            erase_term(existingExitPoint);
            needToRemoveNulls = true;
        } else if (hasExitPoint) {

            // Always refresh the inputs for exit_point(), if there is one.
            update_inputs_for_exit_point(term, existingExitPoint);
        }
    }

    if (needToRemoveNulls)
        block->removeNulls();

#endif
#if 0
        if (term->name == "#return" && !is_output_placeholder(term)) {
            for (int outputIndex=0; outputIndex < term->numInputs(); outputIndex++) {
                Term* returnValue = term->input(outputIndex);
                if (returnValue == NULL)
                    continue;

                //force_term_to_output_to_parent(returnValue);

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
        if (escapingExitLevel != sym_None) {

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
#endif
}

Term* find_intermediate_result_for_output(Term* location, Term* output)
{
    Value description;
    get_output_description(output, &description);
    caValue* descriptionTag = list_get(&description, 0);

    // Control value
    if (as_symbol(descriptionTag) == sym_Control) {
        Block* block = location->owningBlock;
        for (int i = location->index; i >= 0; i--) {
            Term* term = block->get(i);
            if (term == NULL)
                continue;
            if (term->boolProp("control", false))
                return term;
        }
        return NULL;
    }
    
    // Check whether the output's connection is valid at this location
    Term* result = output->input(0);
    if (result != NULL
            && result->owningBlock == output->owningBlock
            && result->index < location->index)
        return result;

    // State output
    if (is_state_input(output))
        return find_open_state_result(location);

    // Nearest with same name
    if (output->name != "")
        return find_name_at(location, output->name.c_str());

    return NULL;
}

static void update_inputs_for_exit_point(Term* exitCall, Term* exitPoint)
{
    // Each input to the exit_point should correspond with an output from this
    // block. Our goal is to capture the "in progress" value for each output.
    
    Block* block = exitPoint->owningBlock;

    set_inputs(exitPoint, TermList());

    for (int i=0;; i++) {

        Term* output = get_output_placeholder(block, i);
        if (output == NULL)
            break;

        // For 'return', check for outputs that are directly given as return() args.
        if (exitCall->function == FUNCS.return_func) {
            if (i < exitCall->numInputs()) {
                set_input(exitPoint, i, exitCall->input(i));
                continue;
            }
        }

        Term* intermediateValue = find_intermediate_result_for_output(exitPoint, output);
        set_input(exitPoint, i, intermediateValue);
    }
}

static void create_implicit_outputs_for_exit_point(Term* exitCall, Term* exitPoint)
{
    Block* block = exitPoint->owningBlock;

    // For a return(), make sure there are enough anonymous outputs.
    int returnOutputs = 0;
    if (exitCall->function == FUNCS.return_func)
        returnOutputs = exitCall->numInputs();

    int existingAnonOutputs = count_anonymous_outputs(block);
    for (int outputIndex = returnOutputs; outputIndex < existingAnonOutputs; outputIndex++) {
        insert_output_placeholder(block, NULL, outputIndex);
    }

    // Named outputs.
    
    // exitLevel value.
}

void control_flow_setup_funcs(Block* kernel)
{
    FUNCS.exit_point =
        import_function(kernel, NULL, "exit_point(any outs :multiple :optional)");

    FUNCS.return_func = import_function(kernel, NULL, "return(any outs :multiple :optional)");
    block_set_evaluation_empty(function_contents(FUNCS.return_func), true);
    as_function(FUNCS.return_func)->formatSource = return_formatSource;

    FUNCS.discard = import_function(kernel, NULL, "discard()");
    block_set_evaluation_empty(function_contents(FUNCS.discard), true);
    as_function(FUNCS.discard)->formatSource = discard_formatSource;

    FUNCS.break_func = import_function(kernel, NULL, "break()");
    block_set_evaluation_empty(function_contents(FUNCS.break_func), true);
    as_function(FUNCS.break_func)->formatSource = break_formatSource;

    FUNCS.continue_func = import_function(kernel, NULL, "continue()");
    block_set_evaluation_empty(function_contents(FUNCS.continue_func), true);
    as_function(FUNCS.continue_func)->formatSource = continue_formatSource;
}

} // namespace circa
