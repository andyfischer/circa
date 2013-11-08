// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "kernel.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "loops.h"
#include "names.h"
#include "stateful_code.h"
#include "string_type.h"
#include "tagged_value.h"
#include "term.h"
#include "term_list.h"

namespace circa {
    
void bytecode_write_output_instructions(caValue* bytecode, Term* caller, Block* block);
static void bytecode_write_local_reference(caValue* bytecode, Block* callingBlock, Term* term);

void bytecode_op_to_string(const char* bc, int* pc, caValue* string)
{
    char op = blob_read_char(bc, pc); 

    switch (op) {
    case bc_Done:
        set_string(string, "done");
        break;
    case bc_Pause:
        set_string(string, "pause");
        break;
    case bc_SetNull:
        set_string(string, "set_null ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_InlineCopy:
        set_string(string, "inline_copy ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_LocalCopy:
        set_string(string, "local_copy ");
        string_append(string, blob_read_int(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_NoOp:
        set_string(string, "noop");
        break;
    case bc_EnterFrame:
        set_string(string, "enter_frame");
        break;
    case bc_LeaveFrame:
        set_string(string, "leave_frame");
        break;
    case bc_PopFrame:
        set_string(string, "pop_frame");
        break;
    case bc_PushFunction:
        set_string(string, "push_function ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushNested:
        set_string(string, "push_nested ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushDynamicMethod:
        set_string(string, "push_dyn_method ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushFuncCall:
        set_string(string, "push_func_call ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushFuncCallImplicit:
        set_string(string, "push_func_call_implicit ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushFuncApply:
        set_string(string, "push_func_apply ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_FireNative:
        set_string(string, "fire_native");
        break;
    case bc_PushCase:
        set_string(string, "push_case ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushLoop:
        set_string(string, "push_loop ");
        string_append(string, blob_read_int(bc, pc));
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_PushWhile:
        set_string(string, "push_while ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushRequire:
        set_string(string, "push_require ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_ExitPoint:
        set_string(string, "exit_point");
        break;
    case bc_Return:
        set_string(string, "return ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_Continue:
        set_string(string, "continue");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_Break:
        set_string(string, "break");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_Discard:
        set_string(string, "discard");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_CaseConditionBool:
        set_string(string, "case_condition_bool ");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_LoopConditionBool:
        set_string(string, "loop_condition_bool ");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_Loop:
        set_string(string, "loop");
        break;
    case bc_IterationDone:
        set_string(string, "iteration_done ");
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_ErrorNotEnoughInputs:
        set_string(string, "error_not_enough_inputs");
        break;
    case bc_ErrorTooManyInputs:
        set_string(string, "error_too_many_inputs");
        break;
    case bc_PushInputFromStack:
        set_string(string, "push_input_from_stack ");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_PushInputNull:
        set_string(string, "push_input_null");
        break;
    case bc_PushInputFromValue:
        set_string(string, "push_input_from_value ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushNonlocalInput:
        set_string(string, "push_nonlocal_input ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PushExplicitState:
        set_string(string, "push_explicit_state ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PopOutput:
        set_string(string, "pop_output ");
        string_append(string, blob_read_int(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PopOutputNull:
        set_string(string, "pop_output_null ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PopOutputsDynamic:
        set_string(string, "pop_outputs_dynamic");
        break;
    case bc_PopExplicitState:
        set_string(string, "pop_explicit_state ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_PopAsModule:
        set_string(string, "pop_as_module ");
        string_append(string, blob_read_int(bc, pc));
        break;
    case bc_MemoizeCheck:
        set_string(string, "memoize_check");
        break;
    case bc_MemoizeSave:
        set_string(string, "memoize_save");
        break;
    case bc_PackState:
        set_string(string, "pack_state ");
        for (int i=0; i < 4; i++) {
            if (i > 0)
                string_append(string, " ");
            string_append(string, blob_read_u16(bc, pc));
        }
        break;
    default:
        set_string(string, "*unrecognized op: ");
        string_append(string, int(op));
    }
}

int bytecode_op_to_term_index(const char* bc, int pc)
{
    char op = blob_read_char(bc, &pc);

    switch (op) {
    case bc_Done:
        return -1;
    case bc_Pause:
        return -1;
    case bc_SetNull:
        return blob_read_int(bc, &pc);
    case bc_InlineCopy:
        return blob_read_int(bc, &pc);
    case bc_LocalCopy:
        return -1;
    case bc_NoOp:
        return -1;
    case bc_EnterFrame:
        return -1;
    case bc_LeaveFrame:
        return -1;
    case bc_PopFrame:
        return -1;
    case bc_PushFunction:
        return blob_read_int(bc, &pc);
    case bc_PushNested:
        return blob_read_int(bc, &pc);
    case bc_PushDynamicMethod:
        return blob_read_int(bc, &pc);
    case bc_PushFuncCall:
        return blob_read_int(bc, &pc);
    case bc_PushFuncCallImplicit:
        return blob_read_int(bc, &pc);
    case bc_PushFuncApply:
        return blob_read_int(bc, &pc);
    case bc_FireNative:
        return -1;
    case bc_PushCase:
        return blob_read_int(bc, &pc);
    case bc_PushLoop:
        return blob_read_int(bc, &pc);
    case bc_PushWhile:
        return blob_read_int(bc, &pc);
    case bc_PushRequire:
        return blob_read_int(bc, &pc);
    case bc_ExitPoint:
    case bc_Return:
    case bc_Continue:
    case bc_Break:
    case bc_Discard:
    case bc_CaseConditionBool:
    case bc_LoopConditionBool:
    case bc_Loop:
    case bc_IterationDone:
    case bc_PushInputFromStack:
    case bc_PushInputNull:
    case bc_PushInputFromValue:
    case bc_PushNonlocalInput:
    case bc_PushExplicitState:
    case bc_ErrorNotEnoughInputs:
    case bc_ErrorTooManyInputs:
    case bc_PopOutput:
    case bc_PopOutputNull:
    case bc_PopOutputsDynamic:
    case bc_PopExplicitState:
    case bc_PopAsModule:
    case bc_MemoizeCheck:
    case bc_MemoizeSave:
    case bc_PackState:
    default:
        return -1;
    }
}

void bytecode_to_string(caValue* bytecode, caValue* string)
{
    std::stringstream strm;

    const char* bc = as_blob(bytecode);
    int pos = 0;

    while (true) {

        int prevPos = pos;

        circa::Value line;
        bytecode_op_to_string(bc, &pos, &line);

        strm << as_cstring(&line) << std::endl;

        switch (blob_read_char(bc, &prevPos)) {
            case bc_Done:
                set_string(string, strm.str().c_str());
                return;
        }

        if (pos >= blob_size(bytecode)) {
            strm << "*error: passed end of buffer" << std::endl;
            set_string(string, strm.str().c_str());
            return;
        }
    }
}

void bytecode_to_string_lines(caValue* bytecode, caValue* lines)
{
    int pos = 0;

    set_list(lines, 0);

    while (pos < blob_size(bytecode)) {

        caValue* line = list_append(lines);
        set_string(line, "[");
        string_append(line, pos);
        string_append(line, "] ");

        circa::Value op;
        bytecode_op_to_string(as_cstring(bytecode), &pos, &op);

        string_append(line, &op);
    }
}

void bytecode_dump(caValue* bytecode)
{
    circa::Value lines;
    bytecode_to_string_lines(bytecode, &lines);

    for (int i=0; i < list_length(&lines); i++) {
        std::cout << as_cstring(list_get(&lines, i)) << std::endl;
    }
}

void bytecode_dump_next_op(const char* bc, int pc)
{
    circa::Value line;
    int lookahead = pc;
    bytecode_op_to_string(bc, &lookahead, &line);
    printf("[%d] %s\n", pc, as_cstring(&line));
}

bool block_contains_memoize(Block* block)
{
    return find_term_with_function(block, FUNCS.memoize) != NULL;
}

void bytecode_write_term_call(caValue* bytecode, Term* term)
{
    INCREMENT_STAT(WriteTermBytecode);

    if (term->function == FUNCS.output) {

        if (term->input(0) == NULL) {
            blob_append_char(bytecode, bc_SetNull);
            blob_append_int(bytecode, term->index);
            return;
        } else {
            blob_append_char(bytecode, bc_InlineCopy);
            blob_append_int(bytecode, term->index);
            return;
        }
    }

    else if (term->function == FUNCS.nonlocal) {
        blob_append_char(bytecode, bc_PushNonlocalInput);
        blob_append_int(bytecode, term->index);
        return;
    }

    else if (term->function == FUNCS.case_condition_bool) {
        blob_append_char(bytecode, bc_CaseConditionBool);
        bytecode_write_local_reference(bytecode, term->owningBlock, term->input(0));
        return;
    }

    else if (term->function == FUNCS.loop_condition_bool) {
        blob_append_char(bytecode, bc_LoopConditionBool);
        bytecode_write_local_reference(bytecode, term->owningBlock, term->input(0));
        return;
    }

    if (is_exit_point(term)) {
        if (term->function == FUNCS.return_func) {
            blob_append_char(bytecode, bc_Return);
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.break_func) {
            blob_append_char(bytecode, bc_Break);
            blob_append_char(bytecode, enclosing_loop_produces_output_value(term));
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.continue_func) {
            blob_append_char(bytecode, bc_Continue);
            blob_append_char(bytecode, enclosing_loop_produces_output_value(term));
            blob_append_int(bytecode, term->index);
        } else if (term->function == FUNCS.discard) {
            blob_append_char(bytecode, bc_Discard);
            blob_append_char(bytecode, enclosing_loop_produces_output_value(term));
            blob_append_int(bytecode, term->index);
        } else {
            internal_error("unrecognized exit point function");
        }

        return;
    }

    if (is_value(term))
        return;

    if (term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated) {
        // These funcs have a nestedContents, but shouldn't be evaluated.
        return;
    }

    // 'referenceTargetBlock' is a block that describes the expected inputs & outputs.
    // It might be the exact block that will actually be pushed (such as for PushNested),
    // or it might just be a block that resembles what will be pushed at runtime (such as
    // for PushCase). It also might not be known at all, in the case of dynamic dispatch
    // (Closure calls and dynamic_method), in which case it will remain NULL.
    Block* referenceTargetBlock = NULL;

    if (term->function == FUNCS.func_call) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushFuncCall);
        blob_append_int(bytecode, term->index);
    }

    else if (is_dynamic_func_call(term)) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushFuncCallImplicit);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.func_apply) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushFuncApply);
        blob_append_int(bytecode, term->index);
    }
    
    else if (term->function == FUNCS.dynamic_method) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushDynamicMethod);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.if_block) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(bytecode, bc_PushCase);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.for_func) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(bytecode, bc_PushLoop);
        blob_append_int(bytecode, term->index);
        blob_append_char(bytecode, loop_produces_output_value(term) ? 0x1 : 0x0);
    }

    else if (term->function == FUNCS.while_loop) {
        referenceTargetBlock = term->nestedContents;
        blob_append_char(bytecode, bc_PushWhile);
        blob_append_int(bytecode, term->index);
    }
    
    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        referenceTargetBlock = function_contents(term->function);
        blob_append_char(bytecode, bc_PushFunction);
        blob_append_int(bytecode, term->index);
    }

    else if (term->function == FUNCS.require) {
        referenceTargetBlock = NULL;
        blob_append_char(bytecode, bc_PushRequire);
        blob_append_int(bytecode, term->index);
    }

    else if (term->nestedContents != NULL) {

        // Otherwise if the term has nested contents, then use it.
        referenceTargetBlock = term->nestedContents;

        if (block_is_evaluation_empty(referenceTargetBlock))
            return;

        blob_append_char(bytecode, bc_PushNested);
        blob_append_int(bytecode, term->index);
    } else {

        // If no other case applies, use the term's function.
        referenceTargetBlock = function_contents(term->function);

        if (block_is_evaluation_empty(referenceTargetBlock))
            return;

        blob_append_char(bytecode, bc_PushFunction);
        blob_append_int(bytecode, term->index);
    }

    bytecode_write_input_instructions(bytecode, term);

    blob_append_char(bytecode, bc_EnterFrame);
    bytecode_write_output_instructions(bytecode, term, referenceTargetBlock);
    blob_append_char(bytecode, bc_PopFrame);
}

void bytecode_write_input_instructions(caValue* bytecode, Term* caller)
{
    for (int i=0; i < caller->numInputs(); i++) {
        Term* input = caller->input(i);

        if (input == NULL) {
            blob_append_char(bytecode, bc_PushInputNull);
        } else if (is_value(input) || input->owningBlock == global_builtins_block()) {
            blob_append_char(bytecode, bc_PushInputFromValue);
            blob_append_int(bytecode, i);
        } else {
            blob_append_char(bytecode, bc_PushInputFromStack);
            bytecode_write_local_reference(bytecode, caller->owningBlock, input);
        }
    }
}

void bytecode_write_output_instructions(caValue* bytecode, Term* caller, Block* block)
{
    if (caller->function == FUNCS.require) {
        blob_append_char(bytecode, bc_PopAsModule);
        blob_append_int(bytecode, caller->index);
        return;
    }

    if (block == NULL) {
        blob_append_char(bytecode, bc_PopOutputsDynamic);
        return;
    }

    int placeholderIndex = 0;

    for (int outputIndex=0;; outputIndex++) {
        Term* output = get_output_term(caller, outputIndex);
        if (output == NULL)
            break;

        if (output->boolProp("state", false)) {
            // Explicit state output.
            blob_append_char(bytecode, bc_PopExplicitState);
            blob_append_int(bytecode, outputIndex);

        } else {
            // Normal output.

            Term* placeholder = get_output_placeholder(block, placeholderIndex);
            if (placeholder == NULL) {
                blob_append_char(bytecode, bc_PopOutputNull);
                blob_append_int(bytecode, outputIndex);
            } else {
                blob_append_char(bytecode, bc_PopOutput);
                blob_append_int(bytecode, placeholderIndex);
                blob_append_int(bytecode, outputIndex);
            }

            placeholderIndex++;
        }
    }
}

static int get_expected_stack_distance(Block* from, Block* to)
{
    int distance = 0;

    while (from != to) {
        // The one case where the block distance doesn't match the stack frame distance
        // is with an if-block. The if-block has a 'parent' block (that contains each
        // condition) which itself does not get a stack frame.
        if (!is_if_block(from))
            distance++;

        from = get_parent_block(from);
        ca_assert(from != NULL);
    }
    return distance;
}

static void bytecode_write_local_reference(caValue* bytecode, Block* callingBlock, Term* term)
{
    blob_append_u16(bytecode, get_expected_stack_distance(callingBlock, term->owningBlock));
    blob_append_u16(bytecode, term->index);
}

static void write_pre_exit_pack_state(caValue* bytecode, Block* block, Term* exitPoint)
{
    // Add PackState ops for each minor block.
    bool anyPackState = false;

    UpwardIterator2 packStateSearch;

    if (exitPoint != NULL) {
        packStateSearch = UpwardIterator2(exitPoint);
        packStateSearch.stopAt(find_block_that_exit_point_will_reach(exitPoint));
    } else {
        packStateSearch = UpwardIterator2(block);
        packStateSearch.stopAt(block);
    }

    for (; packStateSearch; ++packStateSearch) {
        Term* term = packStateSearch.current();

        if (term->function == FUNCS.declared_state) {
            Term* stateResult = NULL;
            if (exitPoint != NULL)
                stateResult = find_name_at(exitPoint, &term->nameValue);
            else
                stateResult = find_name(block, &term->nameValue);

            blob_append_char(bytecode, bc_PackState);
            bytecode_write_local_reference(bytecode, block, term);
            bytecode_write_local_reference(bytecode, block, stateResult);
            anyPackState = true;
        }
    }
}

static void write_loop_finish(caValue* bytecode, Block* block)
{
    // Copy values for looped_inputs.
    for (int i=0;; i++) {
        Term* term = block->get(i);
        if (term->function != FUNCS.looped_input)
            break;

        blob_append_char(bytecode, bc_LocalCopy);
        blob_append_int(bytecode, term->input(1)->index);
        blob_append_int(bytecode, term->index);
    }
}

static void write_block_pre_exit(caValue* bytecode, Block* block, Term* exitPoint)
{
    write_pre_exit_pack_state(bytecode, block, exitPoint);

    if (block_contains_memoize(block))
        blob_append_char(bytecode, bc_MemoizeSave);
}

void bytecode_write_block(caValue* bytecode, Block* block)
{
    if (!is_blob(bytecode))
        set_blob(bytecode, 0);

    bool exitAdded = false;

    if (block_contains_memoize(block))
        blob_append_char(bytecode, bc_MemoizeCheck);

    // Check to just trigger a C override.
    if (get_override_for_block(block) != NULL) {
        blob_append_char(bytecode, bc_FireNative);

    } else {
        
        for (int i=0; i < block->length(); i++) {
            Term* term = block->get(i);
            if (term == NULL)
                continue;

            if (is_exit_point(term) && !is_conditional_exit_point(term)) {
                write_block_pre_exit(bytecode, block, term);
                bytecode_write_term_call(bytecode, term);
                exitAdded = true;
                break;
            }

            if (is_conditional_exit_point(term))
                write_pre_exit_pack_state(bytecode, block, term);

            bytecode_write_term_call(bytecode, term);
        }
    }

    if (!exitAdded)
        write_block_pre_exit(bytecode, block, NULL);

    if (is_while_loop(block)) {
        write_loop_finish(bytecode, block);
        blob_append_char(bytecode, bc_Loop);
    }

    if (is_for_loop(block)) {
        blob_append_char(bytecode, bc_IterationDone);
        blob_append_char(bytecode, loop_produces_output_value(block->owningTerm) ? 0x1 : 0x0);
    }

    blob_append_char(bytecode, bc_Done);
}

} // namespace circa
