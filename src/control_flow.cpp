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
    else
        return sym_None;
}

Block* find_block_that_exit_point_will_reach(Term* term)
{
    ca_assert(is_exit_point(term));

    Block* block = term->owningBlock;

    // Function exit
    if (term->function == FUNCS.return_func) {
        while (is_minor_block(block)) {
            Block* parent = get_parent_block(block);
            if (parent == NULL)
                return block;

            block = parent;
        }
        return block;
    }

    // For-loop exit
    while (!is_for_loop(block)) {
        Block* parent = get_parent_block(block);
        if (parent == NULL)
            return block;

        block = parent;
    }
    return block;
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
            if (is_input_hidden(term, inputIndex) || term->input(inputIndex) == NULL)
                continue;
            if (inputIndex != 0)
                append_phrase(source, ", ", term, sym_None);
            format_source_for_input(source, term, inputIndex, "", "");
        }
    } else {
        format_term_source_default_formatting(source, term);
    }
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

bool has_escaping_control_flow(Term* term)
{
    return term_get_escaping_exit_level(term) != sym_None;
}

void update_derived_inputs_for_exit_point(Term* term)
{
    // Make sure that this exit point includes every block output as an input.
    // The intermediate value might be different at this location.

    Block* block = find_block_that_exit_point_will_reach(term);
    
    for (int i=0;; i++) {
        Term* blockOutput = get_output_placeholder(block, i);
        if (blockOutput == NULL)
            break;

        // Don't touch input if it's explicit.
        if (i < term->numInputs() && !is_input_implicit(term, i))
            continue;

        Term* intermediateValue = find_intermediate_result_for_output(term, blockOutput);

        set_input(term, i, intermediateValue);
        set_input_implicit(term, i, true);
        set_input_hidden(term, i, true);
    }
}

void update_for_control_flow(Block* block)
{
    if (!block_get_bool_prop(block, sym_HasControlFlow, false))
        return;

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        if (is_exit_point(term))
            update_derived_inputs_for_exit_point(term);

        Block* nestedBlock = term->nestedContents;
        if (nestedBlock != NULL && is_minor_block(nestedBlock))
            update_for_control_flow(nestedBlock);
    }
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

void controlFlow_postCompile(Term* term)
{
    // Mark the owning block, and all parent minor blocks, as hasControlFlow.
    Block* block = term->owningBlock;

    while (true) {
        set_bool(block_insert_property(block, sym_HasControlFlow), true);

        if (!is_minor_block(block))
            break;

        block = get_parent_block(block);

        if (block == NULL)
            break;
    }
}

void control_flow_setup_funcs(Block* kernel)
{
    FUNCS.return_func = import_function(kernel, NULL, "return(any outs :multiple :optional)");
    block_set_evaluation_empty(function_contents(FUNCS.return_func), true);
    block_set_format_source_func(function_contents(FUNCS.return_func), return_formatSource);
    block_set_post_compile_func(function_contents(FUNCS.return_func), controlFlow_postCompile);

    FUNCS.discard = import_function(kernel, NULL, "discard(any outs :multiple :optional)");
    block_set_evaluation_empty(function_contents(FUNCS.discard), true);
    block_set_format_source_func(function_contents(FUNCS.discard), discard_formatSource);
    block_set_post_compile_func(function_contents(FUNCS.discard), controlFlow_postCompile);

    FUNCS.break_func = import_function(kernel, NULL, "break(any outs :multiple :optional)");
    block_set_evaluation_empty(function_contents(FUNCS.break_func), true);
    block_set_format_source_func(function_contents(FUNCS.break_func), break_formatSource);
    block_set_post_compile_func(function_contents(FUNCS.break_func), controlFlow_postCompile);

    FUNCS.continue_func = import_function(kernel, NULL, "continue(any outs :multiple :optional)");
    block_set_evaluation_empty(function_contents(FUNCS.continue_func), true);
    block_set_format_source_func(function_contents(FUNCS.continue_func), continue_formatSource);
    block_set_post_compile_func(function_contents(FUNCS.continue_func), controlFlow_postCompile);
}

} // namespace circa
