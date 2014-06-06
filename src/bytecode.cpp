// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "bytecode.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "hashtable.h"
#include "if_block.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "loops.h"
#include "names.h"
#include "native_patch.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "term_list.h"
#include "world.h"

namespace circa {

struct Writer {
    World* world;
    Compiled* compiled;
    int blockIndex;
    caValue* bytecode;
    bool skipEffects;
    bool noSaveState;

    Block* block() {
        return compiled_block(compiled, blockIndex)->block;
    }

    int position() {
        return blob_size(bytecode);
    }
};

static void bc_push_frame(Writer* writer, int termIndex, int inputCount);
static void bc_copy(Writer* writer, Block* top, Term* source, Term* dest, Symbol moveOrCopy);
static void bc_finish_loop_iteration(Writer* writer, Block* block, Symbol exitType);
static void bc_native_call_to_builtin(Writer* writer, const char* name);
static void bytecode_start_term_call(Writer* writer, Term* term, Block* knownBlock);
static void bytecode_write_input_instruction(Writer* writer, Term* term, int inputIndex, Block* knownBlock);
static void bytecode_write_input_instructions(Writer* writer, Term* caller, Block* knownBlock);
static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block);
static void bytecode_write_local_reference(Writer* writer, Block* callingBlock, Term* term);

static void write_pre_exit_pack_state(Writer* writer, Block* block, Term* exitPoint);
static void write_post_term_call(Writer* writer, Term* term);
static void write_block_pre_exit(Writer* writer, Block* block, Term* exitPoint);
static void possibly_write_watch_check(Writer* writer, Term* term);
static caValue* term_effective_value(Writer* writer, Term* term);
Block* get_parent_block_stackwise(Block* block);
int find_register_distance(Block* callingBlock, Term* input);

static void bc_append_local_reference_string(caValue* string, const char* bc, int* pc)
{
    string_append(string, "frame:");
    string_append(string, blob_read_u16(bc, pc));
    string_append(string, " reg:");
    string_append(string, blob_read_u16(bc, pc));
}

struct Jump {
    Writer* writer;
    int position;
    int locationOfDeltaField;

    void jumpHere() {
        u16* delta = (u16*) (as_blob(writer->bytecode) + position + locationOfDeltaField);
        *delta = writer->position() - position;
    }
};

static Jump bc_jump(Writer* writer)
{
    Jump jump;
    jump.writer = writer;
    jump.position = writer->position();
    jump.locationOfDeltaField = 1;

    blob_append_char(writer->bytecode, bc_Jump);
    blob_append_u16(writer->bytecode, 0);
    return jump;
}

static Jump bc_jump_if(Writer* writer, Block* top, Term* condition)
{
    Jump jump;
    jump.writer = writer;
    jump.position = writer->position();
    jump.locationOfDeltaField = 1 + 2 + 2;

    blob_append_char(writer->bytecode, bc_JumpIf);
    bytecode_write_local_reference(writer, top, condition);
    blob_append_u16(writer->bytecode, 0);
    return jump;
}

static Jump bc_jump_if_iterator_done(Writer* writer, Block* top, Term* index, Term* list)
{
    Jump jump;
    jump.writer = writer;
    jump.position = writer->position();
    jump.locationOfDeltaField = 1 + 2 + 2;

    blob_append_char(writer->bytecode, bc_JumpIfIteratorDone);
    blob_append_u16(writer->bytecode, find_register_distance(top, index));
    blob_append_u16(writer->bytecode, 0); // list input
    blob_append_u16(writer->bytecode, 0); // offset

    return jump;
}

static void bc_pop_frame(Writer* writer)
{
    blob_append_u8(writer->bytecode, bc_PopFrame);
}

static void bc_comment(Writer* writer, const char* msg)
{
    blob_append_u8(writer->bytecode, bc_Comment);
    int len = strlen(msg);
    blob_append_u16(writer->bytecode, len);
    string_append_len(writer->bytecode, msg, len);
}

static void bc_set_zero(Writer* writer, Block* top, Term* term)
{
    blob_append_char(writer->bytecode, bc_SetZero);
    blob_append_u16(writer->bytecode, find_register_distance(top, term));
}

static void bc_set_empty_list(Writer* writer, Block* top, Term* term)
{
    blob_append_char(writer->bytecode, bc_SetEmptyList);
    blob_append_u16(writer->bytecode, find_register_distance(top, term));
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
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_SetZero:
        string_append(string, "set_zero ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_SetEmptyList:
        string_append(string, "set_empty_list ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_Increment:
        string_append(string, "increment ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_MoveAppend:
        string_append(string, "move_append ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_InlineCopy:
        string_append(string, "inline_copy ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " ");
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
    case bc_PopFrame:
        string_append(string, "pop_frame");
        break;
    case bc_PopFrameAndPause:
        string_append(string, "pop_frame_and_pause");
        break;
    case bc_ResolveDynamicMethod:
        string_append(string, "resolve_dyn_method ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_ResolveDynamicFuncToClosureCall:
        string_append(string, "resolve_dyn_func_to_closure_call");
        break;
    case bc_NativeCall: {
        string_append(string, "native_call ");
        NativeFuncIndex nativeIndex = blob_read_u32(bc, pc);
        string_append(string, nativeIndex);
        string_append(string, " (");
        caValue* name = get_native_func_name(global_world(), nativeIndex);
        if (name == NULL)
            string_append(string, "!unknown");
        else
            string_append(string, name);
        string_append(string, ")");
        break;
    }
    case bc_PushCase:
        string_append(string, "push_case termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " compiledBlock:");
        string_append(string, blob_read_u32(bc, pc));
        break;
#if 0
    case bc_PushLoop:
        string_append(string, "push_loop termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " compiledBlock:");
        string_append(string, blob_read_u32(bc, pc));
        break;
#endif
    case bc_PushWhile:
        string_append(string, "push_while ");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " compiledBlock:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PushFrame:
        string_append(string, "push_frame termIndex:");
        string_append(string, blob_read_u32(bc, pc));
        string_append(string, " inputCount:");
        string_append(string, blob_read_u8(bc, pc));
        break;
    case bc_PrepareBlock:
        string_append(string, "prepare_block compiledBlock:");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_PrepareBlockUncompiled:
        string_append(string, "prepare_block_uncompiled ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_ResolveClosureCall:
        string_append(string, "resolve_closure_call");
        break;
    case bc_ResolveClosureApply:
        string_append(string, "resolve_closure_apply");
        break;
    case bc_EnterFrame:
        string_append(string, "enter_frame");
        break;
    case bc_EnterFrameNext:
        string_append(string, "enter_frame_next");
        break;
    case bc_FoldIncomingVarargs:
        string_append(string, "fold_incoming_varargs");
        break;
    case bc_CheckInputs:
        string_append(string, "check_inputs");
        break;
    case bc_CopyTermValue:
        string_append(string, "copy_term_value compiledBlock:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " termIndex:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " toRegister:");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_CopyStackValue:
        string_append(string, "copy_stack_value ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_MoveStackValue:
        string_append(string, "move_stack_value ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        string_append(string, " ");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_SetTermRef:
        string_append(string, "set_term_ref compiledBlock:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " termIndex:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " toRegister:");
        string_append(string, blob_read_u8(bc, pc));
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
    case bc_Jump:
        string_append(string, "jump offset:");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_JumpIf:
        string_append(string, "jump_if input:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, "/");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " offset:");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_JumpIfIteratorDone:
        string_append(string, "jump_if_iterator_done index:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " list:");
        string_append(string, blob_read_u16(bc, pc));
        string_append(string, " offset:");
        string_append(string, (i16) blob_read_u16(bc, pc));
        break;
    case bc_JumpToLoopStart:
        string_append(string, "jump_to_loop_start");
        break;
    case bc_FinishIteration:
        string_append(string, "finish_iteration");
        break;
    case bc_FinishFrame:
        string_append(string, "finish_frame");
        break;
    case bc_ErrorNotEnoughInputs:
        string_append(string, "error_not_enough_inputs");
        break;
    case bc_ErrorTooManyInputs:
        string_append(string, "error_too_many_inputs");
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

    case bc_WatchCheck:
        string_append(string, "watch_check ");
        string_append(string, blob_read_u32(bc, pc));
        break;
    case bc_NoteBlockStart:
        string_append(string, "# block_start ");
        string_append(string, blob_read_u16(bc, pc));
        break;
    case bc_Comment: {
        string_append(string, "# ");
        int len = blob_read_u16(bc, pc);
        Value str;
        set_string(&str, bc + *pc, len);
        *pc += len;
        string_append(string, &str);
        break;
    }
    default:
        string_append(string, "*unrecognized op: ");
        string_append(string, int(op));
    }
}

int bytecode_op_to_term_index(const char* bc, int pc)
{
    char op = blob_read_char(bc, &pc);

    switch (op) {
    case bc_PushFrame:
        return blob_read_u32(bc, &pc);
    case bc_ResolveDynamicMethod:
        return blob_read_u32(bc, &pc);
    case bc_PushCase:
        return blob_read_u32(bc, &pc);
#if 0
    case bc_PushLoop:
        return blob_read_u32(bc, &pc);
#endif
    case bc_PushWhile:
        return blob_read_u32(bc, &pc);
    default:
        return -1;
    }
}

void bytecode_to_string(caValue* bytecode, caValue* string)
{
    Value lines;
    bytecode_to_string_lines(as_blob(bytecode), &lines);
    Value newline;
    set_string(&newline, "\n");
    string_join(&lines, &newline, string);
}

void bytecode_to_string_lines(char* bytecode, caValue* lines)
{
    int pos = 0;

    set_list(lines, 0);
    bool indent = false;

    while (true) {

        caValue* line = list_append(lines);
        set_string(line, "[");
        string_append(line, pos);
        string_append(line, "] ");

        if (indent)
            string_append(line, " ");

        int peek = pos;
        char opcode = blob_read_char(bytecode, &peek);

        circa::Value op;
        bytecode_op_to_string(bytecode, &pos, &op);

        string_append(line, &op);

        if (opcode == bc_PushFrame)
            indent = true;
        else if (opcode == bc_EnterFrame)
            indent = false;
        else if (opcode == bc_End)
            return;
    }
}

void bytecode_dump_val(caValue* bytecode)
{
    circa::Value lines;
    bytecode_to_string_lines(as_blob(bytecode), &lines);

    for (int i=0; i < list_length(&lines); i++) {
        printf("%s\n", as_cstring(list_get(&lines, i)));
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
    return get_case_block(ifBlock, caseIndex);
}

Block* find_enclosing_major_block(Block* block)
{
    while (true) {
        if (block == NULL)
            return NULL;

        if (is_major_block(block))
            return block;

        block = get_parent_block(block);
    }
    return NULL;
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

bool compiled_should_ignore_term(Compiled* compiled, Term* term)
{
    if (term == NULL
            || is_value(term)
            || term->function == FUNCS.lambda
            || term->function == FUNCS.block_unevaluated
            || term->function == FUNCS.case_func)

        return true;

    return false;
}

void write_pre_exit_point_for_while_loop(Writer* writer, Term* exitPoint)
{
    Block* callingBlock = exitPoint->owningBlock;
    Block* loopBlock = find_enclosing_loop(callingBlock);

    for (int i=0;; i++) {
        Term* input = get_input_placeholder(loopBlock, i);
        if (input == NULL)
            return;

        Term* latestValue = find_name_at(exitPoint, term_name(input));
        if (latestValue == input)
            continue;

        blob_append_char(writer->bytecode, bc_Copy);
        bytecode_write_local_reference(writer, callingBlock, input);
    }
}

void bc_loop_control_flow(Writer* writer, Term* term)
{
    Block* enclosingLoop = find_enclosing_loop(term->owningBlock);
    Symbol exitType;

    if (term->function == FUNCS.continue_func) {
        bc_comment(writer, "continue");
        exitType = sym_Continue;
    } else if (term->function == FUNCS.break_func) {
        bc_comment(writer, "break");
        exitType = sym_Break;
    } else if (term->function == FUNCS.discard) {
        bc_comment(writer, "discard");
        exitType = sym_Discard;
    } else if (term->function == FUNCS.loop_condition_bool) {
        exitType = sym_Break;
    }

    write_pre_exit_pack_state(writer, term->owningBlock, term);

    // Copy outputs to loop frame's output placeholders.
    
    if (term->function == FUNCS.loop_condition_bool) {
        // When evaluating a loop condition, the block's valid outputs are currently
        // in the input-placeholders, not output-placeholders.
        for (int i=0;; i++) {
            Term* input = get_input_placeholder(enclosingLoop, i);
            if (input == NULL)
                break;

            Term* dest = get_output_placeholder(enclosingLoop, i + 1);
            bc_copy(writer, term->owningBlock, input, dest, sym_Move);
        }
    } else {

        for (int i=0; i < term->numInputs(); i++) {
            Term* input = term->input(i);

            Term* dest = get_output_placeholder(enclosingLoop, i);
            bc_copy(writer, term->owningBlock, input, dest, sym_Move);
        }
    }

    // Pop intermediate frames
    Block* popBlock = term->owningBlock;

    while (popBlock != enclosingLoop) {
        if (exitType != sym_Discard)
            bc_native_call_to_builtin(writer, "#store_frame_state");

        blob_append_char(writer->bytecode, bc_PopFrame);
        popBlock = get_parent_block_stackwise(popBlock);
    }

    if (exitType != sym_Discard)
        bc_native_call_to_builtin(writer, "#store_frame_state");

    bc_finish_loop_iteration(writer, enclosingLoop, exitType);
}

void bc_return(Writer* writer, Term* term)
{
    bc_comment(writer, "return");

    write_pre_exit_pack_state(writer, term->owningBlock, term);

    Block* majorBlock = find_enclosing_major_block(term->owningBlock);

    // Copy outputs to destination frame
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);

        Term* dest = get_output_placeholder(majorBlock, i);

        bc_copy(writer, term->owningBlock, input, dest, sym_Move);
    }

    // Pop intermediate frames
    Block* popBlock = term->owningBlock;

    while (popBlock != majorBlock) {
        bc_native_call_to_builtin(writer, "#store_frame_state");

        blob_append_char(writer->bytecode, bc_PopFrame);
        popBlock = get_parent_block_stackwise(popBlock);
    }

    bc_native_call_to_builtin(writer, "#store_frame_state");
    blob_append_char(writer->bytecode, bc_FinishFrame);
}

void write_term_call(Writer* writer, Term* term)
{
    INCREMENT_STAT(WriteTermBytecode);

    if (compiled_should_ignore_term(writer->compiled, term))
        return;

    if (term->function == FUNCS.output) {

        if (term->input(0) == NULL) {
            blob_append_char(writer->bytecode, bc_SetNull);
            blob_append_u16(writer->bytecode, term->index);
            return;
        } else {
            blob_append_char(writer->bytecode, bc_InlineCopy);
            blob_append_u32(writer->bytecode, term->index);
            bytecode_write_local_reference(writer, term->owningBlock,
                term->input(0));
            return;
        }
    }

    else if (term->function == FUNCS.case_condition_bool) {

        bc_comment(writer, "case condition bool");

        Jump condJump = bc_jump_if(writer, term->owningBlock, term->input(0));
        
        bc_comment(writer, "(case fail, jumping to next case)");

        write_block_pre_exit(writer, term->owningBlock, term);
        bc_pop_frame(writer);

        Block* caseBlock = term->owningBlock;
        ca_assert(is_case_block(caseBlock));
        Block* nextCase = case_condition_get_next_case_block(caseBlock);
        Block* switchBlock = get_parent_block(caseBlock);

        // At the end of every if/switch block, there must be an 'else' case block which
        // contains no case_condition. So here, there must be a non-null nextCase.
        ca_assert(nextCase != NULL);

        bc_push_frame(writer, switchBlock->owningTerm->index, 0);

        blob_append_char(writer->bytecode, bc_PrepareBlock);
        int nextCaseIndex = program_create_entry(writer->compiled, nextCase);
        blob_append_u32(writer->bytecode, nextCaseIndex);

        blob_append_char(writer->bytecode, bc_EnterFrameNext);

        condJump.jumpHere();

        bc_comment(writer, "(case success)");

        return;
    }

    else if (term->function == FUNCS.loop_condition_bool) {
        bc_comment(writer, "loop condition");

        Jump jump = bc_jump_if(writer, term->owningBlock, term->input(0));

        bc_comment(writer, "loop condition is false");

        bc_loop_control_flow(writer, term);

        jump.jumpHere();
        bc_comment(writer, "loop condition is true");
        return;

#if 0
        blob_append_char(writer->bytecode, bc_LoopConditionBool);
        bytecode_write_local_reference(writer, term->owningBlock, term->input(0));
        write_post_term_call(writer, term);
        return;
#endif
    }

    if (is_exit_point(term)) {

#if 0
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
#endif

        if (term->function == FUNCS.return_func) {
            bc_return(writer, term);
        } else if (term->function == FUNCS.break_func
                || term->function == FUNCS.continue_func
                || term->function == FUNCS.discard) {

            bc_loop_control_flow(writer, term);
        } else {
            internal_error("unrecognized exit point function");
        }

        write_post_term_call(writer, term);

        return;
    }

    char inlineOp = get_inline_bc_for_term(writer, term);
    if (inlineOp != 0) {
        blob_append_char(writer->bytecode, bc_PushFrame);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u8(writer->bytecode, 2);
        bytecode_write_input_instructions(writer, term, NULL);
        blob_append_char(writer->bytecode, inlineOp);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;
    }

    if (term->function == FUNCS.func_call || is_dynamic_func_call(term)) {
        // New path
        
        bytecode_start_term_call(writer, term, NULL);

        if (is_dynamic_func_call(term))
            blob_append_char(writer->bytecode, bc_ResolveDynamicFuncToClosureCall);

        blob_append_char(writer->bytecode, bc_ResolveClosureCall);
        blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
        blob_append_char(writer->bytecode, bc_CheckInputs);
        blob_append_char(writer->bytecode, bc_EnterFrame);

        bytecode_write_output_instructions(writer, term, NULL);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);

        return;
    }

    else if (term->function == FUNCS.func_apply) {

        bytecode_start_term_call(writer, term, NULL);

        blob_append_char(writer->bytecode, bc_ResolveClosureApply);
        blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
        blob_append_char(writer->bytecode, bc_CheckInputs);
        blob_append_char(writer->bytecode, bc_EnterFrame);

        bytecode_write_output_instructions(writer, term, NULL);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;
    }
    
    else if (term->function == FUNCS.dynamic_method) {

        bytecode_start_term_call(writer, term, NULL);

        blob_append_char(writer->bytecode, bc_ResolveDynamicMethod);
        blob_append_u32(writer->bytecode, term->index);

        blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
        blob_append_char(writer->bytecode, bc_CheckInputs);
        blob_append_char(writer->bytecode, bc_EnterFrame);

        bytecode_write_output_instructions(writer, term, NULL);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;

    }

    else if (term->function == FUNCS.if_block || term->function == FUNCS.switch_func) {
        Block* firstCaseBlock = get_case_block(term->nestedContents, 0);

        bytecode_start_term_call(writer, term, term->nestedContents);

        blob_append_char(writer->bytecode, bc_PrepareBlock);
        blob_append_u32(writer->bytecode, program_create_entry(writer->compiled, firstCaseBlock));
        
        blob_append_char(writer->bytecode, bc_EnterFrame);

        bytecode_write_output_instructions(writer, term, term->nestedContents);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;
    }

    else if (term->function == FUNCS.for_func) {
        Block* loopBlock = term->nestedContents;

        bc_comment(writer, "for loop");

        bc_push_frame(writer, term->index, block_locals_count(loopBlock));

        bc_copy(writer, loopBlock, term->input(0), get_input_placeholder(loopBlock, 0), sym_Copy);

        // Pass in looped initial values
        for (int i=1;; i++) {
            Term* input = get_input_placeholder(loopBlock, i);
            if (input == NULL)
                break;
                
            bc_copy(writer, loopBlock, input->input(0), input, sym_Copy);
        }
        
        blob_append_char(writer->bytecode, bc_PrepareBlock);
        blob_append_u32(writer->bytecode, program_create_entry(writer->compiled, loopBlock));

        // Initialize iterator
        bc_set_zero(writer, loopBlock, for_loop_find_index(loopBlock));

        // Prepare loop's output
        bc_set_empty_list(writer, loopBlock, term);

        bc_comment(writer, "zero-iteration check");

        Jump loopFinishCheck = bc_jump_if_iterator_done(writer, loopBlock,
            for_loop_find_index(loopBlock), get_input_placeholder(loopBlock, 0));

        bc_comment(writer, "more than zero iterations");


#if 0
        blob_append_u32(writer->bytecode, program_create_entry(writer->compiled,
            for_loop_get_zero_block(term->nestedContents)));

        blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
        blob_append_char(writer->bytecode, bc_CheckInputs);

        //blob_append_char(writer->bytecode, bc_SetZero);
        //blob_append_u16(writer->bytecode, for_loop_find_index(knownBlock)->index);
        #endif

        blob_append_char(writer->bytecode, bc_EnterFrame);
        // No output instructions here (handled inside loop)
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);

        Jump skipZeroIterationBlock = bc_jump(writer);

        loopFinishCheck.jumpHere();
        bc_comment(writer, "zero iterations");

        // move looped values
        for (int i=0;; i++) {
            Term* input = get_input_placeholder(loopBlock, i+1);
            if (input == NULL)
                break;
                
            bc_copy(writer, loopBlock, input, get_extra_output(term, i), sym_Move);
        }

        blob_append_char(writer->bytecode, bc_PopFrame);
        skipZeroIterationBlock.jumpHere();

        return;
    }

    else if (term->function == FUNCS.while_loop) {
        bc_comment(writer, "while");

        Block* loopBlock = term->nestedContents;

        bc_push_frame(writer, term->index, count_input_placeholders(loopBlock));

        // Pass in looped initial values
        for (int i=0;; i++) {
            Term* input = get_input_placeholder(loopBlock, i);
            if (input == NULL)
                break;
                
            bc_copy(writer, loopBlock, input->input(0), input, sym_Copy);
        }

        blob_append_char(writer->bytecode, bc_PrepareBlock);
        blob_append_u32(writer->bytecode, program_create_entry(writer->compiled, loopBlock));

        blob_append_char(writer->bytecode, bc_EnterFrame);
        // No output instructions here (handled inside loop)
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;
#if 0
        blob_append_char(writer->bytecode, bc_PushWhile);
        blob_append_u32(writer->bytecode, term->index);
        blob_append_u32(writer->bytecode,
            program_create_entry(writer->compiled, term->nestedContents));
#endif
    }
    
    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl) {
        // Call the function, not nested contents.
        Block* target = function_contents(term->function);
        if (should_skip_block(writer, target))
            return;
        bytecode_start_term_call(writer, term, target);

        blob_append_char(writer->bytecode, bc_PrepareBlock);
        blob_append_u32(writer->bytecode, program_create_entry(writer->compiled, target));

        blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
        blob_append_char(writer->bytecode, bc_CheckInputs);
        blob_append_char(writer->bytecode, bc_EnterFrame);

        bytecode_write_output_instructions(writer, term, target);
        blob_append_char(writer->bytecode, bc_PopFrame);
        write_post_term_call(writer, term);
        return;
    }

    // Normal function call.
    Block* target = function_contents(term->function);

    if (should_skip_block(writer, target))
        return;

    bytecode_start_term_call(writer, term, target);

    blob_append_char(writer->bytecode, bc_PrepareBlockUncompiled);
    blob_append_u32(writer->bytecode, 0);

    blob_append_char(writer->bytecode, bc_FoldIncomingVarargs);
    blob_append_char(writer->bytecode, bc_CheckInputs);
    blob_append_char(writer->bytecode, bc_EnterFrame);

    bytecode_write_output_instructions(writer, term, target);
    blob_append_char(writer->bytecode, bc_PopFrame);
    write_post_term_call(writer, term);
}

static void write_post_term_call(Writer* writer, Term* term)
{
    possibly_write_watch_check(writer, term);
}

static void possibly_write_watch_check(Writer* writer, Term* term)
{
    int entryIndex = program_find_block_index(writer->compiled, term->owningBlock);
    if (entryIndex == -1)
        return;

    if (!compiled_block(writer->compiled, entryIndex)->hasWatch)
        return;

    // This block has one or more watches, find them.
    for (HashtableIterator it(&writer->compiled->watchByKey); it; ++it) {
        int valueIndex = it.current()->asInt();
        caValue* watch = writer->compiled->cachedValues.index(valueIndex);
        caValue* targetPath = watch->index(1);
        Term* targetTerm = as_term_ref(list_last(targetPath));
        if (targetTerm != term)
            continue;

        blob_append_char(writer->bytecode, bc_WatchCheck);
        blob_append_u32(writer->bytecode, valueIndex);
    }
}

static caValue* find_set_value_hack(Writer* writer, Term* term)
{
    Value termVal;
    set_term_ref(&termVal, term);

    caValue* termSpecificHacks = hashtable_get(&writer->compiled->hacksByTerm, &termVal);
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
        return writer->compiled->cachedValues.index(as_int(setValueHackIndex));

    return term_value(term);
}

Block* get_parent_block_stackwise(Block* block)
{
    block = get_parent_block(block);

    if (is_switch_block(block))
        block = get_parent_block(block);

    return block;
}

int find_register_distance(Block* topBlock, Term* input)
{
    int distance = input->index;
    Block* searchBlock = topBlock;

    while (true) {
        if (searchBlock == input->owningBlock)
            return distance;

        searchBlock = get_parent_block_stackwise(searchBlock);

        distance -= block_locals_count(searchBlock);

        ca_assert(searchBlock != NULL);
    }
    return 0;
}

int find_register_distance_to_new_frame(Block* parentBlock, Term* input)
{
    return find_register_distance(parentBlock, input) - block_locals_count(parentBlock);
}

static void bc_push_frame(Writer* writer, int termIndex, int inputCount)
{
    blob_append_char(writer->bytecode, bc_PushFrame);
    blob_append_u32(writer->bytecode, termIndex);
    blob_append_u8(writer->bytecode, inputCount);
}

static void bc_set_term_ref(Writer* writer, Term* term, int registerIndex)
{
    blob_append_char(writer->bytecode, bc_SetTermRef);
    blob_append_u16(writer->bytecode, program_create_empty_entry(writer->compiled, term->owningBlock));
    blob_append_u16(writer->bytecode, term->index);
    blob_append_u8(writer->bytecode, registerIndex);
}

static void bc_native_call_to_builtin(Writer* writer, const char* name)
{
    Value nameVal;
    set_string(&nameVal, name);
    NativeFuncIndex index = find_native_func_index_by_name(writer->world->builtinPatch, &nameVal);
    ca_assert(index != -1);

    blob_append_char(writer->bytecode, bc_NativeCall);
    blob_append_u32(writer->bytecode, index);
}

static void bc_copy(Writer* writer, Block* top, Term* source, Term* dest, Symbol moveOrCopy)
{
    int destReg = find_register_distance(top, dest);
    if (source == NULL) {
        blob_append_char(writer->bytecode, bc_SetNull);
        blob_append_u16(writer->bytecode, destReg);
    } else if (is_value(source)) {
        int blockIndex = program_create_empty_entry(writer->compiled, source->owningBlock);
        blob_append_char(writer->bytecode, bc_CopyTermValue);
        blob_append_u16(writer->bytecode, blockIndex);
        blob_append_u16(writer->bytecode, source->index);
        blob_append_u16(writer->bytecode, destReg);

    } else {
        if (moveOrCopy == sym_Move)
            blob_append_char(writer->bytecode, bc_MoveStackValue);
        else if (moveOrCopy == sym_Copy)
            blob_append_char(writer->bytecode, bc_CopyStackValue);
        else
            internal_error("unrecognized value for moveOrCopy");

        blob_append_u16(writer->bytecode, find_register_distance(top, source));
        blob_append_u16(writer->bytecode, destReg);
    }
}

static void bc_loop_back_to_start(Writer* writer, Block* block)
{
    // Helper function used in bc_finish_loop_iteration. Don't call directly.

    // Copy rebound outputs back to inputs
    for (int i=0;; i++) {
        Term* input = get_input_placeholder(block, i);
        if (input == NULL)
            break;
        Term* output = input->input(1);
        if (output != NULL)
            bc_copy(writer, block, output, input, sym_Move);
    }

    // Return to start of the loop.
    int blockIndex = program_find_block_index(writer->compiled, block);
    blob_append_char(writer->bytecode, bc_JumpToLoopStart);
}

static void bc_loop_break_out(Writer* writer, Block* block)
{
    // Helper function used in bc_finish_loop_iteration. Don't call directly.

    // Move outputs to caller's extra outputs.
    for (int i=0;; i++) {
        Term* output = get_output_placeholder(block, i+1);
        if (output == NULL)
            break;
        bc_copy(writer, block, output, get_extra_output(block->owningTerm, i), sym_Move);
    }

    blob_append_char(writer->bytecode, bc_FinishFrame);
}

static void bc_finish_loop_iteration(Writer* writer, Block* block, Symbol exitType)
{
    // precondition: loop block is the top frame.
    
    bc_comment(writer, "finish loop iteration");

    Term* indexTerm = NULL;

    if (is_for_loop(block)) {
        // Increment index
        blob_append_char(writer->bytecode, bc_Increment);
        indexTerm = for_loop_find_index(block);
        blob_append_u16(writer->bytecode, find_register_distance(block, indexTerm));
    }

    // Preserve list output
    if (exitType != sym_Discard && is_for_loop(block)) {
        blob_append_char(writer->bytecode, bc_MoveAppend);
        blob_append_u16(writer->bytecode, get_output_placeholder(block, 0)->index);
        blob_append_u16(writer->bytecode, find_register_distance(block, block->owningTerm));
    }

    if (exitType == sym_Break) {

        bc_comment(writer, "break out of loop");
        bc_loop_break_out(writer, block);

    } else if (is_while_loop(block)) {

        bc_comment(writer, "repeat while loop");
        bc_loop_back_to_start(writer, block);

    } else {

        bc_comment(writer, "check if for-loop is finished");
        Jump loopFinishCheck = bc_jump_if_iterator_done(writer, block, indexTerm, get_input_placeholder(block, 0));
        bc_comment(writer, "loop is not finished");
        bc_loop_back_to_start(writer, block);
        loopFinishCheck.jumpHere();
        bc_comment(writer, "loop is finished");
        bc_loop_break_out(writer, block);
    }
}

static void bytecode_write_input_instruction(Writer* writer, Term* term, int inputIndex, Block* knownBlock)
{
    Term* input = term->input(inputIndex);
    Term* placeholder = NULL;

    if (knownBlock != NULL)
        placeholder = get_input_placeholder(knownBlock, inputIndex);

    if (input == NULL) {
        blob_append_char(writer->bytecode, bc_SetNull);
        blob_append_u16(writer->bytecode, inputIndex);
        return;
    }

    // Handle :ref inputs. Only works on statically-known destination blocks.
    if (placeholder != NULL && placeholder->boolProp(sym_Ref, false)) {
        int blockIndex = program_create_empty_entry(writer->compiled, input->owningBlock);
        blob_append_char(writer->bytecode, bc_SetTermRef);
        blob_append_u16(writer->bytecode, blockIndex);
        blob_append_u16(writer->bytecode, input->index);
        blob_append_u8(writer->bytecode, inputIndex);
        return;
    }

    // Hack check, look for a :set_value on this input.
    caValue* setValueIndex = find_set_value_hack(writer, input);
    if (setValueIndex != NULL) {
        blob_append_char(writer->bytecode, bc_CopyCachedValue);
        blob_append_u32(writer->bytecode, as_int(setValueIndex));
        blob_append_u8(writer->bytecode, inputIndex);
        return;
    }

    if (is_value(input) || input->owningBlock == global_builtins_block()) {
        int blockIndex = program_create_empty_entry(writer->compiled, input->owningBlock);

        blob_append_char(writer->bytecode, bc_CopyTermValue);
        blob_append_u16(writer->bytecode, blockIndex);
        blob_append_u16(writer->bytecode, input->index);
        blob_append_u16(writer->bytecode, inputIndex);

    } else {
        blob_append_char(writer->bytecode, bc_CopyStackValue);
        blob_append_u16(writer->bytecode, find_register_distance_to_new_frame(term->owningBlock, input));
        blob_append_u16(writer->bytecode, inputIndex);
    }
}

static void bytecode_start_term_call(Writer* writer, Term* term, Block* knownBlock)
{
    bc_push_frame(writer, term->index, term->numInputs());
    bytecode_write_input_instructions(writer, term, knownBlock);
}

static void bytecode_write_input_instructions(Writer* writer, Term* caller, Block* knownBlock)
{
    if (caller->function == FUNCS.switch_func) {
        // switch's input is used by nested cases.
        return;
    }

    for (int i=0; i < caller->numInputs(); i++)
        bytecode_write_input_instruction(writer, caller, i, knownBlock);
}

static void bytecode_write_output_instructions(Writer* writer, Term* caller, Block* block)
{
    if (block == NULL) {
        blob_append_char(writer->bytecode, bc_PopOutputsDynamic);
        return;
    }

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
        if (!is_switch_block(from))
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

            bc_push_frame(writer, -1, 2);
            bc_set_term_ref(writer, term, 0);

            {
                Term* input = stateResult;
                if (is_value(input) || input->owningBlock == global_builtins_block()) {
                    int blockIndex = program_create_empty_entry(writer->compiled, input->owningBlock);

                    blob_append_char(writer->bytecode, bc_CopyTermValue);
                    blob_append_u16(writer->bytecode, blockIndex);
                    blob_append_u16(writer->bytecode, input->index);
                    blob_append_u16(writer->bytecode, 1);

                } else {
                    blob_append_char(writer->bytecode, bc_CopyStackValue);
                    blob_append_u16(writer->bytecode, find_register_distance_to_new_frame(block, input));
                    blob_append_u16(writer->bytecode, 1);
                }
            }

            bc_native_call_to_builtin(writer, "#save_state_result");
            blob_append_u8(writer->bytecode, bc_PopFrame);

            anyPackState = true;
        }
    }
}

static void write_block_pre_exit(Writer* writer, Block* block, Term* exitPoint)
{
    if (exitPoint && exitPoint->function == FUNCS.discard)
        return;

    write_pre_exit_pack_state(writer, block, exitPoint);

    // Future: Be more smart about when store_frame_state is actually needed.
    if (!writer->noSaveState)
        bc_native_call_to_builtin(writer, "#store_frame_state");
}

void writer_setup(Writer* writer, Compiled* compiled)
{
    ca_assert(compiled->world != NULL);

    writer->compiled = compiled;
    writer->world = compiled->world;
    writer->skipEffects = compiled->skipEffects;
    writer->noSaveState = compiled->noSaveState;
}

void bytecode_write_term_call(Compiled* compiled, caValue* bytecode, Term* term)
{
    Writer writer;
    writer.bytecode = bytecode;
    writer_setup(&writer, compiled);
    set_blob(bytecode, 0);
    write_term_call(&writer, term);
}

Compiled* alloc_program(World* world)
{
    ca_assert(world != NULL);

    Compiled* compiled = new Compiled();
    set_blob(&compiled->bytecode, 0);
    compiled->world = world;
    compiled->blocks = NULL;
    compiled->blockCount = 0;
    set_hashtable(&compiled->blockMap);
    set_hashtable(&compiled->hacksByTerm);
    set_hashtable(&compiled->watchByKey);
    set_list(&compiled->cachedValues);
    return compiled;
}

void free_program(Compiled* compiled)
{
    delete compiled;
}

Block* program_block(Compiled* compiled, int index)
{
    if (index == -1)
        return NULL;
    ca_assert(index < compiled->blockCount);
    return compiled->blocks[index].block;
}

CompiledBlock* compiled_block(Compiled* compiled, int index)
{
    ca_assert(index < compiled->blockCount);
    return &compiled->blocks[index];
}

int program_find_block_index(Compiled* compiled, Block* block)
{
    Value key;
    set_block(&key, block);
    caValue* indexVal = hashtable_get(&compiled->blockMap, &key);
    if (indexVal == NULL)
        return -1;
    return as_int(indexVal);
}

bool has_dynamic_target(Compiled* compiled, Term* term)
{
    return term->function == FUNCS.dynamic_method
        || term->function == FUNCS.func_call
        || term->function == FUNCS.func_apply
        || is_dynamic_func_call(term);
}

void compile_block(Writer* writer)
{
    Block* block = writer->block();

    bc_comment(writer, "block start");

    if (should_skip_block(writer, block)) {
        bc_comment(writer, "(skip block)");
        blob_append_char(writer->bytecode, bc_FinishFrame);
        blob_append_char(writer->bytecode, bc_End);
        return;
    }

    // Check to just trigger a native func.
    int nativePatchIndex = find_native_func_index(writer->world, block);
    if (nativePatchIndex != -1) {
        bc_comment(writer, "(just a native func)");
        blob_append_char(writer->bytecode, bc_NativeCall);
        blob_append_u32(writer->bytecode, nativePatchIndex);
        blob_append_char(writer->bytecode, bc_FinishFrame);
        blob_append_char(writer->bytecode, bc_End);
        return;
    }

    bc_native_call_to_builtin(writer, "#load_frame_state");

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term == NULL)
            continue;

        write_term_call(writer, term);

        if (is_unconditional_exit_point(term)) {
            blob_append_char(writer->bytecode, bc_End);
            return;
        }
    }

    write_block_pre_exit(writer, block, NULL);

    if (is_for_loop(block) || is_while_loop(block)) {
        bc_finish_loop_iteration(writer, block, sym_Continue);
        //blob_append_char(writer->bytecode,
        //    loop_produces_output_value(block->owningTerm) ? 0x1 : 0x0);
    } else {
        blob_append_char(writer->bytecode, bc_FinishFrame);
    }

    blob_append_char(writer->bytecode, bc_End);
}

void compile_block(Compiled* compiled, int blockIndex)
{
    CompiledBlock* cblock = compiled_block(compiled, blockIndex);

    if (cblock->compileInProgress)
        return;
    cblock->compileInProgress = true;

    Value bytecode;

    Writer writer;
    writer.blockIndex = blockIndex;
    writer.bytecode = &bytecode;
    writer_setup(&writer, compiled);
    set_blob(&bytecode, 0);

    compile_block(&writer);

    // 'cblock' pointer is invalidated by compile_block.
    cblock = compiled_block(compiled, blockIndex);

    blob_append_char(&compiled->bytecode, bc_NoteBlockStart);
    blob_append_u16(&compiled->bytecode, blockIndex);

    cblock->bytecodeOffset = string_length(&compiled->bytecode);
    string_append(&compiled->bytecode, &bytecode);
    cblock->compileInProgress = false;
}

int program_create_empty_entry(Compiled* compiled, Block* block)
{
    int existing = program_find_block_index(compiled, block);
    if (existing != -1)
        return existing;

    // Doesn't exist, new create entry.
    int newIndex = compiled->blockCount++;
    compiled->blocks = (CompiledBlock*) realloc(compiled->blocks,
        sizeof(*compiled->blocks) * compiled->blockCount);

    CompiledBlock* entry = &compiled->blocks[newIndex];

    entry->compileInProgress = false;
    entry->block = block;
    entry->hasWatch = false;
    entry->hasState = sym_Unknown;
    entry->bytecodeOffset = -1;

    Value key;
    set_block(&key, block);
    set_int(hashtable_insert(&compiled->blockMap, &key), newIndex);

    return newIndex;
}

int program_create_entry(Compiled* compiled, Block* block)
{
    int index = program_find_block_index(compiled, block);
    if (index == -1)
        index = program_create_empty_entry(compiled, block);

    if (compiled_block(compiled, index)->bytecodeOffset == -1)
        compile_block(compiled, index);

    return index;
}

caValue* program_add_cached_value(Compiled* compiled, int* index)
{
    *index = list_length(&compiled->cachedValues);
    return list_append(&compiled->cachedValues);
}

caValue* program_get_cached_value(Compiled* compiled, int index)
{
    return list_get(&compiled->cachedValues, index);
}

void program_add_watch(Compiled* compiled, caValue* key, caValue* path)
{
    int cachedIndex = 0;
    caValue* watch = program_add_cached_value(compiled, &cachedIndex);

    // Watch value is: [key, path, observation]
    set_list(watch, 3);
    watch->set_element(0, key);
    watch->set_element(1, path);
    watch->set_element_null(2);

    // Update watchByKey
    set_int(hashtable_insert(&compiled->watchByKey, key), cachedIndex);

    Term* targetTerm = as_term_ref(list_last(path));

    // Update CompiledBlock
    int blockIndex = program_create_empty_entry(compiled, targetTerm->owningBlock);
    compiled_block(compiled, blockIndex)->hasWatch = true;
}

caValue* program_get_watch_observation(Compiled* compiled, caValue* key)
{
    caValue* index = hashtable_get(&compiled->watchByKey, key);
    if (index == NULL)
        return NULL;

    caValue* watch = list_get(&compiled->cachedValues, as_int(index));

    return watch->index(2);
}

void program_set_hackset(Compiled* compiled, caValue* hackset)
{
    for (int i=0; i < list_length(hackset); i++) {
        caValue* hacksetElement = hackset->index(i);
        if (symbol_eq(hacksetElement, sym_no_effect))
            compiled->skipEffects = true;
        else if (symbol_eq(hacksetElement, sym_no_save_state))
            compiled->noSaveState = true;
        else if (first_symbol(hacksetElement) == sym_set_value) {
            caValue* setValueTarget = hacksetElement->index(1);
            caValue* setValueNewValue = hacksetElement->index(2);

            caValue* termHacks = hashtable_insert(&compiled->hacksByTerm, setValueTarget);
            if (!is_hashtable(termHacks))
                set_hashtable(termHacks);

            int cachedValueIndex = 0;
            set_value(program_add_cached_value(compiled, &cachedValueIndex), setValueNewValue);
            set_int(hashtable_insert_symbol_key(termHacks, sym_set_value), cachedValueIndex);
        } else if (first_symbol(hacksetElement) == sym_watch) {
            caValue* watchKey = hacksetElement->index(1);
            caValue* watchPath = hacksetElement->index(2);
            program_add_watch(compiled, watchKey, watchPath);
        }
    }
}

void program_erase(Compiled* compiled)
{
    set_blob(&compiled->bytecode, 0);
    free(compiled->blocks);
    compiled->blocks = NULL;
    compiled->blockCount = 0;
    set_hashtable(&compiled->blockMap);
    compiled->noSaveState = false;
    compiled->skipEffects = false;
    set_hashtable(&compiled->hacksByTerm);
    set_hashtable(&compiled->watchByKey);
    set_list(&compiled->cachedValues);
}

void compiled_to_string(Compiled* compiled, Value* out)
{
    if (!is_string(out))
        set_string(out, "");

    string_append(out, "[Compiled]\n");
    string_append(out, "  Hackset: ");
    string_append(out, &compiled->hackset);
    string_append(out, "\n");

    for (int blockIndex=0; blockIndex < compiled->blockCount; blockIndex++) {
        CompiledBlock* cblock = compiled_block(compiled, blockIndex);
        string_append(out, "  [CompiledBlock ");
        string_append(out, blockIndex);
        string_append(out, ", blockId: ");
        string_append(out, cblock->block->id);
        string_append(out, ", blockName: ");
        string_append(out, block_name(cblock->block));
        string_append(out, "]\n");

        if (cblock->bytecodeOffset == -1) {
            string_append(out, "    (no bytecode)\n");
            continue;
        }

        char* bytecode = as_blob(&compiled->bytecode);
        int pos = cblock->bytecodeOffset;

        while (true) {
            string_append(out, "    [");
            string_append(out, pos);
            string_append(out, "] ");
            char op = bytecode[pos];
            bytecode_op_to_string(bytecode, &pos, out);
            string_append(out, "\n");

            if (op == bc_End)
                break;
        }
    }
}

void compiled_dump(Compiled* compiled)
{
    Value str;
    compiled_to_string(compiled, &str);
    printf("%s\n", as_cstring(&str));
}

} // namespace circa
