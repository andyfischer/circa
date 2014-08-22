// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "inspection.h"
#include "interpreter.h"
#include "function.h"
#include "importing.h"
#include "inspection.h"
#include "kernel.h"
#include "string_type.h"
#include "switch.h"
#include "term.h"

namespace circa {

static Symbol max_exit_level(Symbol left, Symbol right);

bool is_exit_point(Term* term)
{
    return term->function == FUNCS.return_func
        || term->function == FUNCS.break_func
        || term->function == FUNCS.continue_func
        || term->function == FUNCS.discard
        || term->function == FUNCS.case_condition_bool
        || term->function == FUNCS.loop_condition_bool;
}

bool is_unconditional_exit_point(Term* term)
{
    return term->function == FUNCS.return_func
        || term->function == FUNCS.break_func
        || term->function == FUNCS.continue_func
        || term->function == FUNCS.discard;
}

bool is_conditional_exit_point(Term* term)
{
    return term->function == FUNCS.case_condition_bool;
}

bool is_exit_point_that_handles_rebinding(Term* term)
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

    // 'return' exits to nearest major block.
    if (term->function == FUNCS.return_func) {
        while (is_minor_block(block)) {
            Block* parent = get_parent_block(block);
            if (parent == NULL)
                return block;

            block = parent;
        }
        return block;
    }

    // 'case_condition_bool' exits the current if-block.
    if (term->function == FUNCS.case_condition_bool)
        return get_parent_block(term->owningBlock);

    // Otherwise, exit to nearest for-loop.
    while (!is_for_loop(block) && !is_while_loop(block)) {
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

void create_output_from_minor_block(Block* block, Value* description)
{
    if (is_case_block(block)) {
        Block* ifBlock = get_parent_block(block);
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
        Term* inputResult = get_output_placeholder(block, i);
        if (inputResult == NULL)
            break;

        // Don't touch input if it's explicit.
        if (i < term->numInputs() && !is_input_implicit(term, i))
            continue;

        Term* intermediateValue = find_intermediate_result_for_output(term, inputResult);

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

        if (is_exit_point_that_handles_rebinding(term))
            update_derived_inputs_for_exit_point(term);

        Block* nestedBlock = term->nestedContents;
        if (nestedBlock != NULL && is_minor_block(nestedBlock))
            update_for_control_flow(nestedBlock);
    }
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

} // namespace circa
