// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "hashtable.h"
#include "if_block.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "loops.h"
#include "names.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "term_list.h"

namespace circa {

struct Writer {
    Stack* stack;
    BytecodeCache* cache;
    caValue* bytecode;
    bool skipEffects;
    bool noSaveState;
};

static void bytecode_write_input_instruction(Writer* writer, Term* input);
static void bytecode_write_input_instructions(Writer* writer, Term* caller);
static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block);
static void bytecode_write_local_reference(Writer* writer, Block* callingBlock, Term* term);

static void write_post_term_call(Writer* writer, Term* term);
static void possibly_write_watch_check(Writer* writer, Term* term);
static caValue* term_effective_value(Writer* writer, Term* term);

static void bc_append_local_reference_string(caValue* string, const char* bc, int* pc)
{
    string_append(string, "frame:");
    string_append(string, blob_read_u16(bc, pc));
    string_append(string, " reg:");
    string_append(string, blob_read_u16(bc, pc));
}

void bytecode_op_to_string(const char* bc, int* pc, caValue* string)
{
    if (!is_string(string))
        set_string(string, "");

    char op = blob_read_char(bc, pc); 

    switch (op) {
    case bc_End:
        string_append(string, "end");
        break;
    case bc_Pause:
        string_append(string, "pause");
        break;
    case bc_SetNull:
        string_append(string, "set_null ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_InlineCopy:
        string_append(string, "inline_copy ");
        string_append(string, blob_read_u32(bc, pc));
        bc_append_local_reference_string(string, bc, pc);
        break;
    case bc_LocalCopy:
        string_append(string, "local_copy ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Copy:
        string_append(string, "copy to ");
        bc_append_local_reference_string(string, bc, pc);
        break;
    case bc_NoOp:
        string_append(string, "noop");
        break;
    case bc_EnterFrame:
        string_append(string, "enter_frame");
        break;
    case bc_PopFrame:
        string_append(string, "pop_frame");
        break;
    case bc_PopFrameAndPause:
        string_append(string, "pop_frame_and_pause");
        break;
    case bc_PushFunction:
        string_append(string, "push_function termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " bc:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushDynamicMethod:
        string_append(string, "push_dyn_method ");
        string_append(string, blob_read_u32(bc, pc));

        for (int i=0; i < c_methodCacheCount; i++) {
            MethodCallSiteCacheLine* line = (MethodCallSiteCacheLine*) &bc[*pc];
            *pc += sizeof(*line);
            string_append(string, "\n  ");
            string_append(string, line->typeId);
            string_append(string, " -> ");
            string_append(string, line->blockIndex);
        }
        break;
    case bc_PushFuncCall:
        string_append(string, "push_func_call ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushFuncApply:
        string_append(string, "push_func_apply ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_FireNative:
        string_append(string, "fire_native");
        break;
    case bc_PushCase:
        string_append(string, "push_case termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " blockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushLoop:
        string_append(string, "push_loop termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " blockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " zeroBlockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_PushWhile:
        string_append(string, "push_while ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " blockIndex:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushRequire:
        string_append(string, "push_require ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_ExitPoint:
        string_append(string, "exit_point");
        break;
    case bc_Return:
        string_append(string, "return ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Continue:
        string_append(string, "continue");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Break:
        string_append(string, "break");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Discard:
        string_append(string, "discard");
        string_append(string, blob_read_char(bc, pc) ? " :loop_output": "");
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_Continue2:
        string_append(string, "continue2");
        break;
    case bc_Break2:
        string_append(string, "break2");
        break;
    case bc_CaseConditionBool:
        string_append(string, "case_condition_bool input:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, "/");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " nextBlockBc:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_LoopConditionBool:
        string_append(string, "loop_condition_bool input:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, "/");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_Loop:
        string_append(string, "loop");
        break;
    case bc_FinishIteration:
        string_append(string, "finish_iteration ");
        if (blob_read_char(bc, pc))
            string_append(string, " :with_output");
        else
            string_append(string, " :no_output");
        break;
    case bc_FinishBlock:
        string_append(string, "finish_block");
        break;
    case bc_DynamicTermEval:
        string_append(string, "dynamic_term_eval ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopRequire:
        string_append(string, "pop_require");
        break;
    case bc_ErrorNotEnoughInputs:
        string_append(string, "error_not_enough_inputs");
        break;
    case bc_ErrorTooManyInputs:
        string_append(string, "error_too_many_inputs");
        break;
    case bc_InputFromStack:
        string_append(string, "input_from_stack ");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_InputNull:
        string_append(string, "input_null");
        break;
    case bc_InputFromValue:
        string_append(string, "input_from_value ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushNonlocalInput:
        string_append(string, "push_nonlocal_input ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_InputFromBlockRef:
        string_append(string, "input_from_block_ref ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_InputFromCachedValue:
        string_append(string, "input_from_cached_value ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushExplicitState:
        string_append(string, "push_explicit_state ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopOutput:
        string_append(string, "pop_output ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_char(bc, pc));
        break;
    case bc_PopOutputNull:
        string_append(string, "pop_output_null ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PopOutputsDynamic:
        string_append(string, "pop_outputs_dynamic");
        break;
    case bc_PopExplicitState:
        string_append(string, "pop_explicit_state ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_SetFrameOutput:
        string_append(string, "set_frame_output ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_MemoizeCheck:
        string_append(string, "memoize_check");
        break;
    case bc_MemoizeSave:
        string_append(string, "memoize_save");
        break;
    case bc_SetInt:
        string_append(string, "set_int ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_SetFloat:
        string_append(string, "set_float ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
        string_append_f(string, blob_read_float(bc, pc));
        break;
    case bc_SetTermValue:
        string_append(string, "set_value ");
        string_append(string, blob_read_u32(bc, pc));
        break;

    #define INLINE_MATH_CASE(op, str) \
        case op: \
            string_append(string, str " "); \
            string_append(string, blob_read_u32(bc, pc)); \
            break;

    INLINE_MATH_CASE(bc_Addf, "add_f");
    INLINE_MATH_CASE(bc_Addi, "add_i");
    INLINE_MATH_CASE(bc_Subi, "sub_i");
    INLINE_MATH_CASE(bc_Subf, "sub_f");
    INLINE_MATH_CASE(bc_Multi, "mult_i");
    INLINE_MATH_CASE(bc_Multf, "mult_f");
    INLINE_MATH_CASE(bc_Divi, "div_i");
    INLINE_MATH_CASE(bc_Divf, "div_f");
    INLINE_MATH_CASE(bc_Eqf, "eq_f");
    INLINE_MATH_CASE(bc_Neqf, "neq_f");
    INLINE_MATH_CASE(bc_EqShallow, "eq_shallow");
    INLINE_MATH_CASE(bc_NeqShallow, "neq_shallow");

    #undef INLINE_MATH_CASE

    case bc_PackState:
        string_append(string, "pack_state ");
        for (int i=0; i < 4; i++) {
            if (i > 0)
                string_append(string, " ");
            string_append(string, blob_read_u16(bc, pc));
        }
        break;
    case bc_WatchCheck:
        string_append(string, "watch_check ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    default:
        string_append(string, "*unrecognized op: ");
        string_append(string, int(op));
    }
}

int bytecode_op_to_term_index(const char* bc, int pc)
{
    char op = blob_read_char(bc, &pc);

    switch (op) {
    case bc_Pause:
        return -1;
    case bc_SetNull:
        return blob_read_u32(bc, &pc);
    case bc_InlineCopy:
        return -1;
    case bc_LocalCopy:
        return -1;
    case bc_NoOp:
        return -1;
    case bc_EnterFrame:
        return -1;
    case bc_PopFrame:
        return -1;
    case bc_PushFunction:
        return blob_read_u32(bc, &pc);
    case bc_PushDynamicMethod:
        return blob_read_u32(bc, &pc);
    case bc_PushFuncCall:
        return blob_read_u32(bc, &pc);
    case bc_PushFuncApply:
        return blob_read_u32(bc, &pc);
    case bc_FireNative:
        return -1;
    case bc_PushCase:
        return blob_read_u32(bc, &pc);
    case bc_PushLoop:
        return blob_read_u32(bc, &pc);
    case bc_PushWhile:
        return blob_read_u32(bc, &pc);
    case bc_PushRequire:
        return blob_read_u32(bc, &pc);
    case bc_ExitPoint:
    case bc_Return:
    case bc_Continue:
    case bc_Break:
    case bc_Discard:
    case bc_CaseConditionBool:
    case bc_LoopConditionBool:
    case bc_Loop:
    case bc_FinishIteration:
    case bc_InputFromStack:
    case bc_InputNull:
    case bc_InputFromValue:
    case bc_PushNonlocalInput:
    case bc_InputFromBlockRef:
    case bc_InputFromCachedValue:
    case bc_PushExplicitState:
    case bc_ErrorNotEnoughInputs:
    case bc_ErrorTooManyInputs:
    case bc_PopOutput:
    case bc_PopOutputNull:
    case bc_PopOutputsDynamic:
    case bc_PopExplicitState:
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
            case bc_End:
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
        bytecode_op_to_string(as_blob(bytecode), &pos, &op);

        string_append(line, &op);
    }
}

void bytecode_dump_val(caValue* bytecode)
{
    circa::Value lines;
    bytecode_to_string_lines(bytecode, &lines);

    for (int i=0; i < list_length(&lines); i++) {
        std::cout << as_cstring(list_get(&lines, i)) << std::endl;
    }
}

void bytecode_dump(char* data)
{
    int pc = 0;
    while (data[pc] != bc_End)
        bytecode_dump_next_op(data, &pc);
}

void bytecode_dump_next_op(const char* bc, int *pc)
{
    int originalPc = *pc;
    circa::Value line;
    bytecode_op_to_string(bc, pc, &line);
    printf("[%d] %s\n", originalPc, as_cstring(&line));
}

bool block_contains_memoize(Block* block)
{
    return find_term_with_function(block, FUNCS.memoize) != NULL;
}

bool block_contains_literal_symbol(Block* block, Symbol symbol)
{
    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (!is_value(term))
            continue;
        if (symbol_eq(term_value(term), symbol))
            return true;
    }
    return false;
}

static bool should_skip_block(Writer* writer, Block* block)
{
    if (writer->skipEffects && block_contains_literal_symbol(block, sym_effect))
        return true;
    if (block_is_evaluation_empty(block))
        return true;

    return false;
}

static Type* term_type(Writer* writer, Term* term)
{
    if (is_value(term))
        return term_effective_value(writer, term)->value_type;

    return declared_type(term);
}

static bool type_is_int_or_float(Type* type)
{
    return type == TYPES.float_type || type == TYPES.int_type;
}

static char get_inline_bc_for_term(Writer* writer, Term* term)
{
    if (term->numInputs() != 2)
        return 0;  // Only have inline BC for binary functions.
    if (term->input(0) == NULL || term->input(1) == NULL)
        return 0;

    Type* leftType = term_type(writer, term->input(0));
    Type* rightType = term_type(writer, term->input(1));

    bool bothInts = leftType == TYPES.int_type && rightType == TYPES.int_type;
    bool bothFloaty = type_is_int_or_float(leftType) && type_is_int_or_float(rightType);

#define CASE(func, whenInts, whenFloats) \
    if (term->function == func) { \
        if (bothInts) \
            return whenInts; \
        if (bothFloaty) \
            return whenFloats; \
        return 0; \
    }

    CASE(FUNCS.add, bc_Addi, bc_Addf);
    CASE(FUNCS.add_i, bc_Addi, 0);
    CASE(FUNCS.add_f, bc_Addf, bc_Addf);
    CASE(FUNCS.sub, bc_Subi, bc_Subf);
    CASE(FUNCS.sub_i, bc_Subi, 0);
    CASE(FUNCS.sub_f, bc_Subf, bc_Subf);
    CASE(FUNCS.mult, bc_Multi, bc_Multf);
    CASE(FUNCS.div, bc_Divf, bc_Divf);
    CASE(FUNCS.div_f, bc_Divf, bc_Divf);
    CASE(FUNCS.div_i, bc_Divi, 0);
    CASE(FUNCS.equals, bc_EqShallow, bc_Eqf);
    CASE(FUNCS.not_equals, bc_NeqShallow, bc_Neqf);

#undef CASE

    return 0;
}

Block* case_condition_get_next_case_block(Block* currentCase)
{
    Block* ifBlock = get_parent_block(currentCase);
    int caseIndex = case_block_get_index(currentCase) + 1;
    return if_block_get_case(ifBlock, caseIndex);
}

Block* find_enclosing_loop(Block* block)
{
    while (true) {
        if (block == NULL)
            return NULL;

        if (is_while_loop(block) || is_for_loop(block))
            return block;

        block = get_parent_block(block);
    }
    return NULL;
}

void write_pre_exit_point_for_while_loop(Writer* writer, Term* exitPoint)
{
    Block* callingBlock = exitPoint->owningBlock;
    Block* loopBlock = find_enclosing_loop(callingBlock);

    for (int i=0; i < loopBlock->length(); i++) {
        Term* input = loopBlock->get(i);
        if (input->function != FUNCS.looped_input)
            return;

        Term* latestValue = find_name_at(exitPoint, term_name(input));
        if (latestValue == input)
            continue;

        blob_append_char(writer->bytecode, bc_Copy);
        bytecode_write_local_reference(writer, callingBlock, input);
        bytecode_write_input_instruction(writer, latestValue);
    }
}

void write_term_call(Writer* writer, Term* term)
{
    INCREMENT_STAT(WriteTermBytecode);

    if (term->function == FUNCS.output) {

        if (term->input(0) == NULL) {
            blob_append_char(writer->bytecode, bc_SetNull);
            blob_append_u32(writer->bytecode, term->index);
            return;
        } else {
            blob_append_char(writer->bytecode, bc_InlineCopy);
            blob_append_u32(writer->bytecode, term->index);
            bytecode_write_local_reference(writer, term->owningBlock,
                term->input(0));
            return;
        }
    }

    else if (term->function == FUNCS.nonlocal) {
        blob_append_char(writer->bytecode, bc_PushNonlocalInput);
        blob_append_u32(writer->bytecode, term->index);

        blob_append_char(writer->bytecode, bc_EnterFrame);

        blob_append_char(writer->bytecode, bc_PopOutput);
        blob_append_u32(writer->bytecode, 0);
        blob_append_u32(writer->bytecode, 0);
        blob_append_char(writer->bytecode, 0);
        blob_append_char(writer->bytecode, bc_PopFrame);

        write_post_term_call(writer, term);

        return;
    }

    else if (term->function == FUNCS.case_condition_bool) {
        blob_append_char(writer->bytecode, bc_CaseConditionBool);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));

        Block* nextCase = case_condition_get_next_case_block(term->owningBlock);
        int nextBlockBcIndex = stack_bytecode_create_entry(writer->stack, nextCase);
        blob_append_u32(writer->bytecode, nextBlockBcIndex);
        write_post_term_call(writer, term);
        return;
    }

    else if (term->function == FUNCS.loop_condition_bool) {
        blob_append_char(writer->bytecode, bc_LoopConditionBool);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));
        write_post_term_call(writer, term);
        return;
    }

    if (is_exit_point(term)) {
        Block* enclosingLoop = find_enclosing_loop(term->owningBlock);

        if (is_while_loop(enclosingLoop)) {
            if (term->function == FUNCS.return_func) {
                blob_append_char(writer->bytecode, bc_Return);
                blob_append_u32(writer->bytecode, term->index);
            } else if (term->function == FUNCS.break_func) {
                write_pre_exit_point_for_while_loop(writer, term);
                blob_append_char(writer->bytecode, bc_Break2);
            } else if (term->function == FUNCS.continue_func) {
                write_pre_exit_point_for_while_loop(writer, term);
                blob_append_char(writer->bytecode, bc_Continue2);
            }

        } else {
            if (term->function == FUNCS.return_func) {
                blob_append_char(writer->bytecode, bc_Return);
                blob_append_u32(writer->bytecode, term->index);
            } else if (term->function == FUNCS.break_func) {
                blob_append_char(writer->bytecode, bc_Break);
                blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
                blob_append_u32(writer->bytecode, term->index);
            } else if (term->function == FUNCS.continue_func) {
                blob_append_char(writer->bytecode, bc_Continue);
                blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
                blob_append_u32(writer->bytecode, term->index);
            } else if (term->function == FUNCS.discard) {
                blob_append_char(writer->bytecode, bc_Discard);
                blob_append_char(writer->bytecode, enclosing_loop_produces_output_value(term));
                blob_append_u32(writer->bytecode, term->index);
            } else {
                internal_error("unrecognized exit point function");
            }
        }
        write_post_term_call(writer, term);

        return;
    }

    if (is_value(term))
        return;

    char inlineOp = get_inline_bc_for_term(writer, term);
    if (inlineOp != 0) {
        blob_append_char(writer->bytecode, inlineOp);
        blob_append_u32(writer->bytecode, term->index);
        bytecode_write_input_instructions(writer, term);
        write_post_term_call(writer, term);
        return;
    }

    if (term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated) {
        // These funcs have a nestedContents, but shouldn't be evaluated.
        return;
    }

    // 'staticallyKnownBlock' is a block that describes the expected inputs & outputs.
    // It might be the exact block that will actually be pushed (such as for PushNested),
    // or it might just be a block that resembles what will be pushed at runtime (such as
    // for PushCase). It also might not be known at all, in the case of dynamic dispatch
    // (Closure calls and dynamic_method), in which case it will remain NULL.
    Block* staticallyKnownBlock = NULL;

    if (term->function == FUNCS.func_call) {
        staticallyKnownBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncCall);
        blob_append_u32(writer->bytecode, term->index);
    }

    else if (is_dynamic_func_call(term)) {
        staticallyKnownBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncCall);
        blob_append_u32(writer->bytecode, term->index);
    }

    else if (term->function == FUNCS.func_apply) {
        staticallyKnownBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushFuncApply);
        blob_append_u32(writer->bytecode, term->index);
    }
    
    else if (term->function == FUNCS.dynamic_method) {
        staticallyKnownBlock = NULL;
        blob_append_char(writer->bytecode, bc_PushDynamicMethod);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_space(writer->bytecode, c_methodCacheSize);
    }

    else if (term->function == FUNCS.dynamic_term_eval) {
        staticallyKnownBlock = NULL;
        blob_append_char(writer->bytecode, bc_DynamicTermEval);
        blob_append_u32(writer->bytecode, term->index);
    }

    else if (term->function == FUNCS.if_block) {
        staticallyKnownBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushCase);
        blob_append_u32(writer->bytecode, term->index);
        Block* firstCaseBlock = if_block_get_case(term->nestedContents, 0);
        u32 blockIndex = stack_bytecode_create_entry(writer->stack, firstCaseBlock);
        blob_append_u32(writer->bytecode, blockIndex);
    }

    else if (term->function == FUNCS.case_func) {
        // Handled by the if_block case.
        return;
    }

    else if (term->function == FUNCS.for_func) {
        staticallyKnownBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushLoop);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, stack_bytecode_create_entry(writer->stack, term->nestedContents));
        blob_append_u32(writer->bytecode, stack_bytecode_create_entry(writer->stack,
            for_loop_get_zero_block(term->nestedContents)));
        blob_append_char(writer->bytecode, loop_produces_output_value(term) ? 0x1 : 0x0);
    }

    else if (term->function == FUNCS.while_loop) {
        staticallyKnownBlock = term->nestedContents;
        blob_append_char(writer->bytecode, bc_PushWhile);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode,
            stack_bytecode_create_entry(writer->stack, term->nestedContents));
    }
    
    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        staticallyKnownBlock = function_contents(term->function);
        if (should_skip_block(writer, staticallyKnownBlock))
            return;
        blob_append_char(writer->bytecode, bc_PushFunction);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, 0xffffffff);
    }

    else if (term->nestedContents != NULL) {

        internal_error("bytecode_write_term_call: function has nestedContents but isn't recognized as special case");

    } else {

        // If no other case applies, use the term's function.
        staticallyKnownBlock = function_contents(term->function);

        if (should_skip_block(writer, staticallyKnownBlock))
            return;

        blob_append_char(writer->bytecode, bc_PushFunction);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode, 0xffffffff);
    }

    bytecode_write_input_instructions(writer, term);

    blob_append_char(writer->bytecode, bc_EnterFrame);
    bytecode_write_output_instructions(writer, term, staticallyKnownBlock);
    blob_append_char(writer->bytecode, bc_PopFrame);
    write_post_term_call(writer, term);
}

static void write_post_term_call(Writer* writer, Term* term)
{
    possibly_write_watch_check(writer, term);
}

static void possibly_write_watch_check(Writer* writer, Term* term)
{
    StackBlock* stackBlock = stack_bytecode_find_entry(writer->stack, term->owningBlock);
    if (stackBlock == NULL || !stackBlock->hasWatch)
        return;

    // This block has one or more watches, find them.
    for (HashtableIterator it(&writer->stack->bytecode.watchByKey); it; ++it) {
        int valueIndex = it.current()->asInt();
        caValue* watch = writer->stack->bytecode.cachedValues.element(valueIndex);
        caValue* targetPath = watch->element(1);
        Term* targetTerm = as_term_ref(list_last(targetPath));
        if (targetTerm != term)
            continue;

        blob_append_char(writer->bytecode, bc_WatchCheck);
        blob_append_u32(writer->bytecode, valueIndex);
    }
}

void bytecode_write_input_instruction_block_ref(Writer* writer, Term* input)
{
    blob_append_char(writer->bytecode, bc_InputFromBlockRef);
    blob_append_u32(writer->bytecode,
        stack_bytecode_create_empty_entry(writer->stack, input->owningBlock));
    blob_append_u32(writer->bytecode, input->index);
}

static caValue* find_set_value_hack(Writer* writer, Term* term)
{
    Value termVal;
    set_term_ref(&termVal, term);

    caValue* termSpecificHacks = hashtable_get(&writer->cache->hacksByTerm, &termVal);
    if (termSpecificHacks == NULL)
        return NULL;

    caValue* desiredValueIndex = hashtable_get_symbol_key(termSpecificHacks, sym_set_value);
    if (desiredValueIndex == NULL)
        return NULL;

    return desiredValueIndex;
}

static caValue* term_effective_value(Writer* writer, Term* term)
{
    caValue* setValueHackIndex = find_set_value_hack(writer, term);
    if (setValueHackIndex != NULL)
        return writer->stack->bytecode.cachedValues.element(as_int(setValueHackIndex));

    return term_value(term);
}

static void bytecode_write_input_instruction(Writer* writer, Term* input)
{
    if (input == NULL) {
        blob_append_char(writer->bytecode, bc_InputNull);
        return;
    }

    // Hack check, look for a :set_value on this input.
    caValue* setValueIndex = find_set_value_hack(writer, input);
    if (setValueIndex != NULL) {
        blob_append_char(writer->bytecode, bc_InputFromCachedValue);
        blob_append_u32(writer->bytecode, as_int(setValueIndex));
        return;
    }

    if (is_value(input) || input->owningBlock == global_builtins_block()) {
        blob_append_char(writer->bytecode, bc_InputFromValue);
        blob_append_u32(writer->bytecode,
            stack_bytecode_create_empty_entry(writer->stack, input->owningBlock));
        blob_append_u32(writer->bytecode, input->index);
    } else {
        bytecode_write_input_instruction_block_ref(writer, input);
    }
}

static void bytecode_write_input_instructions(Writer* writer, Term* caller)
{
    if (is_dynamic_func_call(caller))
        bytecode_write_input_instruction_block_ref(writer, caller->function);

    if (caller->function == FUNCS.while_loop) {
        Block* block = caller->nestedContents;
        for (int i=0; i < block->length(); i++) {
            Term* input = block->get(i);
            if (input->function != FUNCS.looped_input)
                break;

            bytecode_write_input_instruction(writer, input->input(0));
        }
        return;
    }

    for (int i=0; i < caller->numInputs(); i++) {
        Term* input = caller->input(i);
        bytecode_write_input_instruction(writer, input);
    }
}

static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block)
{
    if (block == NULL) {
        blob_append_char(writer->bytecode, bc_PopOutputsDynamic);
        return;
    }

    if (is_while_loop(block))
        return;

    int placeholderIndex = 0;

    for (int outputIndex=0;; outputIndex++) {
        Term* output = get_output_term(caller, outputIndex);
        if (output == NULL)
            break;

        // Normal output.

        Term* placeholder = get_output_placeholder(block, placeholderIndex);
        if (placeholder == NULL) {
            blob_append_char(writer->bytecode, bc_PopOutputNull);
            blob_append_u32(writer->bytecode, outputIndex);
        } else {
            blob_append_char(writer->bytecode, bc_PopOutput);
            blob_append_u32(writer->bytecode, placeholderIndex);
            blob_append_u32(writer->bytecode, outputIndex);

            blob_append_char(writer->bytecode,
                placeholder->boolProp(sym_ExplicitType, false) ? 1 : 0);
        }

        placeholderIndex++;
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

static void bytecode_write_local_reference(Writer* writer, Block* callingBlock, Term* term)
{
    blob_append_u16(writer->bytecode, get_expected_stack_distance(callingBlock, term->owningBlock));
    blob_append_u16(writer->bytecode, term->index);
}

static void write_pre_exit_pack_state(Writer* writer, Block* block, Term* exitPoint)
{
    if (writer->noSaveState)
        return;

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

            blob_append_char(writer->bytecode, bc_PackState);
            bytecode_write_local_reference(writer, block, term);
            bytecode_write_local_reference(writer, block, stateResult);
            anyPackState = true;
        }
    }
}

static void write_loop_finish(Writer* writer, Block* block)
{
    // Copy values for looped_inputs.
    for (int i=0;; i++) {
        Term* looped_input = block->get(i);
        if (looped_input->function != FUNCS.looped_input)
            break;

        blob_append_char(writer->bytecode, bc_Copy);
        bytecode_write_local_reference(writer, block, looped_input);
        bytecode_write_input_instruction(writer, looped_input->input(1));
    }
}

static void write_block_pre_exit(Writer* writer, Block* block, Term* exitPoint)
{
    write_pre_exit_pack_state(writer, block, exitPoint);

    if (block_contains_memoize(block))
        blob_append_char(writer->bytecode, bc_MemoizeSave);
}

void write_block(Writer* writer, Block* block)
{
    bool exitAdded = false;

    if (block_contains_memoize(block))
        blob_append_char(writer->bytecode, bc_MemoizeCheck);

    if (!should_skip_block(writer, block)) {

        // Check to just trigger a C override.
        if (get_override_for_block(block) != NULL) {
            blob_append_char(writer->bytecode, bc_FireNative);

        } else {
            
            for (int i=0; i < block->length(); i++) {
                Term* term = block->get(i);
                if (term == NULL)
                    continue;

                if (is_exit_point(term) && !is_conditional_exit_point(term)) {
                    write_block_pre_exit(writer, block, term);
                    write_term_call(writer, term);
                    exitAdded = true;
                    break;
                }

                if (is_conditional_exit_point(term))
                    write_pre_exit_pack_state(writer, block, term);

                write_term_call(writer, term);
            }
        }
    }

    if (!exitAdded)
        write_block_pre_exit(writer, block, NULL);

    if (is_while_loop(block)) {
        write_loop_finish(writer, block);
        blob_append_char(writer->bytecode, bc_Loop);
    }

    if (is_for_loop(block)) {
        blob_append_char(writer->bytecode, bc_FinishIteration);
        blob_append_char(writer->bytecode,
            loop_produces_output_value(block->owningTerm) ? 0x1 : 0x0);
    } else {
        blob_append_char(writer->bytecode, bc_FinishBlock);
    }

    blob_append_char(writer->bytecode, bc_End);
}

void writer_setup_from_stack(Writer* writer, Stack* stack)
{
    writer->stack = stack;
    writer->cache = &stack->bytecode;
    writer->skipEffects = stack->bytecode.skipEffects;
    writer->noSaveState = stack->bytecode.noSaveState;
}

void bytecode_write_term_call(Stack* stack, caValue* bytecode, Term* term)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup_from_stack(&writer, stack);
    set_blob(bytecode, 0);
    write_term_call(&writer, term);
}

void bytecode_write_block(Stack* stack, caValue* bytecode, Block* block)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup_from_stack(&writer, stack);
    set_blob(bytecode, 0);
    write_block(&writer, block);
}

} // namespace circa
