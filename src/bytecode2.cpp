// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "bytecode2.h"
#include "closures.h"
#include "debug.h"
#include "hashtable.h"
#include "inspection.h"
#include "list.h"
#include "loops.h"
#include "kernel.h"
#include "modules.h"
#include "native_patch.h"
#include "string_type.h"
#include "switch.h"
#include "symbols.h"
#include "tagged_value.h"
#include "term.h"
#include "type.h"
#include "vm.h"

#define TRACE_EXECUTION 0

/*
 
# Stack Frame Layout #

During a function call, the stack slots are:

       [-2 ]        - ret_pc, the addr to jump to on return
       [-1 ]        - ret_top, the stackTop to restore on return
top -> [ 0 ]        - output, the one output slot
       [1 ... N+1]  - N input slot(s), each located at (top + 1 + index)
       [N+1 ...]    - the function's temporaries

*/

namespace circa {
    
Value* append_const(Bytecode* bc, int* index);
void append_liveness(Bytecode* bc, Term* term, int slot);
bool block_needs_no_evaluation(Bytecode* bc, Block* block);
Value* load_const(Bytecode* bc, int slot);
void write_term(Bytecode* bc, Term* term);
void write_normal_call(Bytecode* bc, Term* term);
int find_compiled_major_block(Bytecode* bc, Block* block);
void close_conditional_case(Bytecode* bc, Block* block);
void loop_advance_iterator(Bytecode* bc, Block* loop);
void loop_move_locals_back_to_starting_slots(Bytecode* bc, Block* loop, Term* atTerm);
void loop_preserve_iteration_result(Bytecode* bc, Block* loop);
void save_declared_state(Bytecode* bc, Block* block, Term* atTerm);
void close_state_frame(Bytecode* bc, Block* block, Term* atTerm);
void set_subroutine_output(Bytecode* bc, Block* block, Term* result);

Bytecode* new_bytecode(VM* vm)
{
    Bytecode* bc = (Bytecode*) malloc(sizeof(Bytecode));
    initialize_null(&bc->unresolved);
    initialize_null(&bc->minorBlockInfo);
    initialize_null(&bc->blockToAddr);
    set_list(&bc->unresolved, 0);
    set_hashtable(&bc->minorBlockInfo);
    set_hashtable(&bc->blockToAddr);
    bc->metadata = NULL;
    bc->metadataSize = 0;
    bc->slotCount = 0;
    bc->consts.init();
    bc->opCount = 0;
    bc->ops = NULL;
    bc->vm = vm;
    return bc;
}

void free_bytecode(Bytecode* bc)
{
    if (bc == NULL)
        return;
    bc->consts.clear();
    set_null(&bc->minorBlockInfo);
    set_null(&bc->unresolved);
    free(bc->ops);
    free(bc->metadata);
}

int reserve_slots(Bytecode* bc, int count)
{
    int first = bc->slotCount;
    bc->slotCount += count;
    return first;
}

int reserve_new_frame_slots(Bytecode* bc, int inputCount)
{
    // Reserve slots for a function call. Returns the first slot (the function's output)
    int top = bc->slotCount + 2;
    bc->slotCount += inputCount + 3;
    return top;
}

Value* get_const(Bytecode* bc, int index)
{
    return bc->consts[index];
}

Value* append_const(Bytecode* bc, int* index)
{
    *index = bc->consts.size;
    bc->consts.growBy(1);
    return bc->consts[*index];
}

void grow_ops(Bytecode* bc, int newSize)
{
    bc->opCount = newSize;
    bc->ops = (Op*) realloc(bc->ops, sizeof(Op) * newSize);
}

void grow_metadata(Bytecode* bc, int newSize)
{
    bc->metadataSize = newSize;
    bc->metadata = (BytecodeMetadata*) realloc(bc->metadata, sizeof(BytecodeMetadata) * newSize);
}

int append_op(Bytecode* bc)
{
    int addr = bc->opCount;
    grow_ops(bc, bc->opCount + 1);
    return addr;
}

int append_op(Bytecode* bc, u8 opcode, u16 a = 0, u16 b = 0, u16 c = 0)
{
    ca_assert(opcode != 0);
    int addr = append_op(bc);
    Op* op = &bc->ops[addr];
    op->opcode = opcode;
    op->a = a;
    op->b = b;
    op->c = c;
    return addr;
}

BytecodeMetadata* append_metadata(Bytecode* bc, u8 mopcode, int slot)
{
    int pos = bc->metadataSize;
    grow_metadata(bc, bc->metadataSize + 1);
    BytecodeMetadata* md = &bc->metadata[pos];
    md->mopcode = mopcode;
    md->addr = bc->opCount;
    md->slot = slot;
    md->term = NULL;
    md->block = NULL;
    md->related_maddr = 0;
    return md;
}

void append_liveness(Bytecode* bc, Term* term, int slot)
{
    append_metadata(bc, mop_term_live, slot)->term = term;
}

int find_live_slot(Bytecode* bc, Term* term, int addr = -1, bool allowNull = false)
{
    for (int i=bc->metadataSize-1; i >= 0; i--) {
        BytecodeMetadata md = bc->metadata[i];
        if (addr != -1 && addr < md.addr)
            continue;

        if (md.mopcode == mop_term_live && md.term == term)
            return md.slot;
    }

    if (allowNull)
        return -1;

    internal_error("find_live_slot: term is not live");
    return -1;
}

void load_term(Bytecode* bc, Term* term, int slot)
{
    Value* foundOverride = hashtable_get_term_key(&bc->vm->termOverrides, term);
    if (foundOverride != NULL) {
        load_const(bc, slot)->set(foundOverride);
        return;
    }

    if (term == NULL) {
        append_op(bc, op_set_null, slot);
        return;
    }

    if (term->function == FUNCS.break_func
            || term->function == FUNCS.continue_func
            || term->function == FUNCS.discard
            || term->function == FUNCS.return_func) {
        // No output value
        append_op(bc, op_set_null, slot);
        return;
    }

    int liveSlot = find_live_slot(bc, term, -1, true);

    if (liveSlot != -1) {
        if (liveSlot != slot)
            append_op(bc, op_copy, liveSlot, slot);
        return;
    }

    if (should_use_term_value(term)) {
        int constIndex;
        copy(term_value(term), append_const(bc, &constIndex));
        append_op(bc, op_load_const, constIndex, slot);
        return;
    }

    internal_error("load_term: term is not live or loadable");
}

int load_or_find_term(Bytecode* bc, Term* term)
{
    int liveSlot = find_live_slot(bc, term, -1, true);

    if (liveSlot != -1)
        return liveSlot;

    int slot = reserve_slots(bc, 1);
    load_term(bc, term, slot);
    return slot;
}

void append_unresolved(Bytecode* bc, int addr, Symbol type)
{
    bc->unresolved.append()->set_hashtable()
        ->set_field_int(s_addr, addr)
        ->set_field_sym(s_type, type);
}

void collect_unresolved(Bytecode* bc, Symbol ofType, Value* toList, int afterAddr)
{
    toList->set_list(0);

    for (int i=0; i < bc->unresolved.length(); i++) {
        Value* unresolved = bc->unresolved.index(i);
        if (unresolved->field(s_type)->as_s() != ofType)
            continue;

        if (afterAddr > 0 && unresolved->field(s_addr)->as_i() < afterAddr)
            continue;

        move(unresolved, toList->append());
    }

    list_remove_nulls(&bc->unresolved);
}

void collect_unresolved_next_case(Bytecode* bc, Block* block, Value* toList)
{
    toList->set_list(0);

    for (int i=0; i < bc->unresolved.length(); i++) {
        Value* unresolved = bc->unresolved.index(i);
        if (unresolved->field(s_type)->as_s() != s_next_case)
            continue;
        if (unresolved->field(s_block)->asBlock() != block)
            continue;

        move(unresolved, toList->append());
    }

    list_remove_nulls(&bc->unresolved);
}

void call(Bytecode* bc, int top, int count, Block* target)
{
    int existingAddr = find_compiled_major_block(bc, target);

    if (existingAddr != -1) {
        append_op(bc, op_call, top, count, existingAddr);
    } else {
        int constIndex;
        set_block(append_const(bc, &constIndex), target);
        append_op(bc, op_uncompiled_call, top, count, constIndex);
    }
}

void comment(Bytecode* bc, Value* val)
{
    int index;
    copy(val, append_const(bc, &index));
    append_op(bc, op_comment, index);
}

void comment(Bytecode* bc, const char* str)
{
    int index;
    append_const(bc, &index)->set_string(str);
    append_op(bc, op_comment, index);
}

Value* load_const(Bytecode* bc, int slot)
{
    int constIndex;
    Value* value = append_const(bc, &constIndex);
    append_op(bc, op_load_const, constIndex, slot);
    return value;
}

void cast_fixed_type(Bytecode* bc, int slot, Type* type)
{
    int constIndex;
    set_type(append_const(bc, &constIndex), type);
    append_op(bc, op_cast_fixed_type, slot, constIndex);
}

bool should_write_state_header(Bytecode* bc, Block* block)
{
    // No state in while-loop. Maybe temp, maybe permanant.
    if (is_while_loop(block))
        return false;
    return block_has_state(block) != s_no;
}

void write_state_header(Bytecode* bc, int keySlot)
{
    int frameSlots = reserve_slots(bc, 4);
    append_op(bc, op_push_state_frame, frameSlots, keySlot);
}

void write_state_header_with_term_name(Bytecode* bc, Term* term)
{
    int slot = reserve_slots(bc, 1);
    copy(unique_name(term), load_const(bc, slot));
    write_state_header(bc, slot);
}

void end_major_frame(Bytecode* bc)
{
    int blockStartMaddr = mop_find_active_mopcode(bc, mop_major_block_start, -1);
    //ca_assert(blockStartMaddr != -1);
    append_metadata(bc, mop_major_block_end, 0)->related_maddr = blockStartMaddr;
}

bool both_inputs_are_int(Term* term)
{
    return term->numInputs() == 2
        && term->input(0) != NULL
        && term->input(0)->type == TYPES.int_type
        && term->input(1) != NULL
        && term->input(1)->type == TYPES.int_type;
}

static bool type_is_int_or_float(Type* type)
{
    return type == TYPES.float_type || type == TYPES.int_type;
}

bool use_inline_bytecode(Bytecode* bc, Term* term)
{
    if (term->numInputs() != 2)
        return false;

    Type* leftType = declared_type(term->input(0));
    Type* rightType = declared_type(term->input(1));
    bool bothInts = leftType == TYPES.int_type && rightType == TYPES.int_type;
    bool bothFloaty = type_is_int_or_float(leftType) && type_is_int_or_float(rightType);

    u8 opcode = 0;

    if (bothInts) {
        if (term->function == FUNCS.add)
            opcode = op_add_i;
        if (term->function == FUNCS.sub)
            opcode = op_sub_i;
        if (term->function == FUNCS.mult)
            opcode = op_mult_i;

    } else if (term->function == FUNCS.add_i) {
        opcode = op_add_i;
    } else if (term->function == FUNCS.sub_i) {
        opcode = op_sub_i;
    //} else if (term->function == FUNCS.mult_i) {
    //    opcode = op_mult_i;
    } else if (term->function == FUNCS.div_i) {
        opcode = op_div_i;
    }

    if (opcode == 0)
        return false;

    int top = reserve_slots(bc, 3);
    load_term(bc, term->input(0), top);
    load_term(bc, term->input(1), top+1);

    append_op(bc, opcode, top, top+1, top+2);
    append_liveness(bc, term, top+2);
    return true;
}

void dynamic_method(Bytecode* bc, Term* term)
{
#if DEBUG
    Value msg;
    set_string(&msg, "dynamic method, slow lookup of: ");
    string_append(&msg, term->getProp(s_MethodName));
    comment(bc, &msg);
#endif

    int constIndex;
    Value* nameLocation = append_const(bc, &constIndex);
    nameLocation->set_list(2);
    nameLocation->index(0)->set(term->getProp(s_MethodName));
    nameLocation->index(1)->set_term(term);

    int inputCount = term->numInputs();
    int top = reserve_new_frame_slots(bc, inputCount);

    for (int i=0; i < inputCount; i++)
        load_term(bc, term->input(i), top + 1 + i);
    
    append_op(bc, op_dyn_method, top, inputCount, constIndex);
    append_liveness(bc, term, top);
}

void write_loop(Bytecode* bc, Block* loop)
{
    comment(bc, "loop start");

    Term* callingTerm = loop->owningTerm;
    if (should_write_state_header(bc, loop))
        write_state_header_with_term_name(bc, callingTerm);
        
    Value blockVal;
    set_block(&blockVal, loop);
    Value* blockInfo = bc->minorBlockInfo.insert_val(&blockVal)->set_hashtable();

    bool produceOutput = term_is_observable(callingTerm);
    blockInfo->insert(s_produceOutput)->set_bool(produceOutput);

    // Load initial values into loop body
    int firstLocalSlot = reserve_slots(bc, count_input_placeholders(loop));
    blockInfo->insert(s_slot)->set_int(firstLocalSlot);

    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(loop, i);
        if (placeholder == NULL)
            break;

        int slot = firstLocalSlot + i;
        load_term(bc, placeholder->input(0), slot);
        append_liveness(bc, placeholder, slot);
    }

    if (is_for_loop(loop)) {
        // Load initial iterator value
        Term* iterator = loop_find_iterator(loop);
        int iteratorSlot = reserve_slots(bc, 1);
        load_term(bc, iterator->input(0), iteratorSlot);
        append_liveness(bc, iterator, iteratorSlot);
        blockInfo->insert(s_iterator)->set_int(iteratorSlot);
    }

    int outputSlot = -1;
    if (produceOutput) {
        outputSlot = reserve_new_frame_slots(bc, 1);
        append_op(bc, op_load_i, 0, outputSlot + 1);
        call(bc, outputSlot, 1, FUNCS.blank_list->nestedContents);
        blockInfo->insert(s_outputSlot)->set_int(outputSlot);
    }
    
    // Start loop
    int loopStart = bc->opCount;

    Term* advanceTerm = NULL;

    // Done check
    Term* doneTerm = NULL;
    if (is_for_loop(loop)) {
        comment(bc, "done check");
        doneTerm = loop_find_done_call(loop);
        advanceTerm = loop_find_iterator_advance(loop);
        write_term(bc, doneTerm);
        int addr = append_op(bc, op_jif, find_live_slot(bc, doneTerm));
        append_unresolved(bc, addr, s_break);
    }

    // Fetch key
    Term* keyTerm = NULL;
    if (is_for_loop(loop)) {
        comment(bc, "loop key");
        Term* keyTerm = loop_find_key(loop);
        write_term(bc, keyTerm);
        if (should_write_state_header(bc, loop))
            write_state_header(bc, load_or_find_term(bc, keyTerm));
    }
    
    // Contents
    for (int i=0; i < loop->length(); i++) {
        Term* term = loop->get(i);

        if (term == doneTerm || term == keyTerm || term == advanceTerm)
            continue;

        write_term(bc, term);
    }

    // Prepare for next iteration.

    close_state_frame(bc, loop, NULL);
    loop_advance_iterator(bc, loop);
    loop_move_locals_back_to_starting_slots(bc, loop, NULL);
    loop_preserve_iteration_result(bc, loop);

    comment(bc, "jump back to loop start");
    append_op(bc, op_jump, 0, 0, loopStart);

    int loopFin = bc->opCount;

    // Collect unresolved jumps
    // 'break' jumps to end, 'continue' jumps to start.
    Value unresolvedContinue;
    collect_unresolved(bc, s_continue, &unresolvedContinue, 0);
    for (int i=0; i < unresolvedContinue.length(); i++) {
        Value* unresolved = unresolvedContinue.index(i);
        int addr = unresolved->field(s_addr)->as_i();
        bc->ops[addr].c = loopStart;
    }
    
    Value unresolvedBreaks;
    collect_unresolved(bc, s_break, &unresolvedBreaks, loopStart);
    for (int i=0; i < unresolvedBreaks.length(); i++) {
        Value* unresolved = unresolvedBreaks.index(i);
        int addr = unresolved->field(s_addr)->as_i();
        bc->ops[addr].c = loopFin;
    }

    // Update liveness
    if (produceOutput)
        append_liveness(bc, callingTerm, outputSlot);

    for (int i=1;; i++) {
        Term* termOutput = get_output_term(callingTerm, i);
        if (termOutput == NULL)
            break;
        Term* placeholder = get_input_placeholder(loop, i - 1);
        append_liveness(bc, termOutput, find_live_slot(bc, placeholder));
    }

    comment(bc, "loop fin");

    if (should_write_state_header(bc, loop))
        append_op(bc, op_pop_state_frame);
}


void loop_advance_iterator(Bytecode* bc, Block* loop)
{
    comment(bc, "loop iterator advance");
    Term* advanceTerm = loop_find_iterator_advance(loop);
    if (is_for_loop(loop)) {
        int iteratorSlot = bc->minorBlockInfo.block_key(loop)->field(s_iterator)->as_i();
        write_term(bc, advanceTerm);
        load_term(bc, advanceTerm, iteratorSlot);
    }
}

void loop_move_locals_back_to_starting_slots(Bytecode* bc, Block* loop, Term* atTerm)
{
    comment(bc, "move looped values");
    int firstLocalSlot = bc->minorBlockInfo.block_key(loop)->field(s_slot)->as_i();

    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(loop, i);
        if (placeholder == NULL)
            break;
        
        Term* finalValue;
        if (atTerm == NULL)
            finalValue = placeholder->input(1);
        else {
            Value position; set_term_ref(&position, atTerm);
            finalValue = find_name_at(&position, term_name(placeholder));
        }

        // TODO: Modify building to just connect the input_placeholder directly to this term.
        if (finalValue->function == FUNCS.output)
            finalValue = finalValue->input(0);

        load_term(bc, finalValue, firstLocalSlot + i);
    }
}

void loop_preserve_iteration_result(Bytecode* bc, Block* loop)
{
    bool produceOutput = bc->minorBlockInfo.block_key(loop)->field(s_produceOutput)->as_b();

    if (produceOutput) {
        int outputSlot = bc->minorBlockInfo.block_key(loop)->field(s_outputSlot)->as_i();
        comment(bc, "preserve iteration result");
        Term* result = get_output_placeholder(loop, 0)->input(0);

        // TODO: Modify building to just connect the input_placeholder directly to this term.
        if (result->function == FUNCS.output)
            result = result->input(0);

        int top = reserve_new_frame_slots(bc, 2);
        append_op(bc, op_copy, outputSlot, top + 1);
        load_term(bc, result, top + 2);
        call(bc, top, 2, FUNCS.list_append->nestedContents);
        append_op(bc, op_copy, top, outputSlot);
    }
}

void loop_condition_bool(Bytecode* bc, Term* term)
{
    comment(bc, "loop condition");
    int slot = reserve_slots(bc, 1);
    load_term(bc, term->input(0), slot);
    int addr = append_op(bc, op_jnif, slot);
    append_unresolved(bc, addr, s_break);
}

void write_normal_call(Bytecode* bc, Term* term)
{
#if DEBUG
    Value commentStr;
    commentStr.set_string("call to: ");
    string_append(&commentStr, term_name(term->function));
    comment(bc, &commentStr);
#endif
    Block* target = term->function->nestedContents;

    if (block_needs_no_evaluation(bc, target)) {
        int top = reserve_new_frame_slots(bc, 0);
        comment(bc, "block needs no evaluation");
        append_liveness(bc, term, top);
        return;
    }

    int inputCount = term->numInputs();
    int top = reserve_new_frame_slots(bc, inputCount);
    
    for (int i=0; i < inputCount; i++) {
        int inputSlot = top + 1 + i;
        load_term(bc, term->input(i), inputSlot);
    }

    if (count_closure_upvalues(target) != 0) {
        load_term(bc, term->function, top);
        append_op(bc, op_func_call_d, top, inputCount);
    } else {
        call(bc, top, inputCount, target);
    }

    append_liveness(bc, term, top);
}

void write_conditional_case(Bytecode* bc, Block* block, int conditionIndex)
{
    // Update jump from previous case
    int caseStartAddr = bc->opCount;

    Value unresolved;
    collect_unresolved_next_case(bc, get_parent_block(block), &unresolved);

    for (int i=0; i < unresolved.length(); i++) {
        int addr = unresolved.index(i)->field(s_addr)->as_s();
        bc->ops[addr].c = caseStartAddr;
    }

    // Check condition
    Term* condition = case_find_condition(block);
    if (condition != NULL) {
        int conditionSlot = reserve_slots(bc, 1);
        load_term(bc, condition, conditionSlot);
        if (declared_type(condition) != TYPES.bool_type)
            cast_fixed_type(bc, conditionSlot, TYPES.bool_type);
        int addr = append_op(bc, op_jnif, conditionSlot);
        bc->unresolved.append()->set_hashtable()
            ->set_field_int(s_addr, addr)
            ->set_field_sym(s_type, s_next_case)
            ->insert(s_block)->set_block(get_parent_block(block));
    }

    comment(bc, "case");

    if (should_write_state_header(bc, block)) {
        int slot = reserve_slots(bc, 1);
        set_int(load_const(bc, slot), conditionIndex);
        write_state_header(bc, slot);
    }

    // Case contents
    for (int i=0; i < block->length(); i++) {
        write_term(bc, block->get(i));

        if (block->get(i)->function == FUNCS.return_func) {
            comment(bc, "case fin (return)");
            return;
        }
    }

    close_conditional_case(bc, block);

    // Jump to switch-finish.
    int addr = append_op(bc, op_jump, 0, 0, 0);
    append_unresolved(bc, addr, s_conditional_done);
}

void close_conditional_case(Bytecode* bc, Block* block)
{
    close_state_frame(bc, block, NULL);

    comment(bc, "case move outputs");

    Block* parentBlock = get_parent_block(block);
    int firstOutputSlot = bc->minorBlockInfo.block_key(parentBlock)->field(s_slot)->as_i();

    // Move outputs
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL)
            break;
        Term* localResult = placeholder->input(0);

        // TODO: Fix block building, looks like there are cases where one output_placeholder
        // is connected to another.
        if (localResult && localResult->function == FUNCS.output)
            localResult = localResult->input(0);

        // TODO: Fix building, case_condition_bool should not be a localResult
        if (localResult && localResult->function == FUNCS.case_condition_bool)
            localResult = NULL;

        load_term(bc, localResult, firstOutputSlot + i);
    }

    comment(bc, "case fin");
}

void write_conditional_chain(Bytecode* bc, Block* block)
{
    comment(bc, "conditional start");
    int startAddr = bc->opCount;

    if (should_write_state_header(bc, block))
        write_state_header_with_term_name(bc, block->owningTerm);

    // Assign output slots. Each conditional block will put its respective outputs here.
    int outputCount = count_output_placeholders(block);
    int firstOutputSlot = reserve_slots(bc, outputCount);

    Value blockVal;
    set_block(&blockVal, block);
    bc->minorBlockInfo.insert_val(&blockVal)->set_hashtable()->insert(s_slot)->set_int(firstOutputSlot);

    int conditionIndex = 0;
    for (int i=0; i < block->length(); i++)
        if (block->get(i)->function == FUNCS.case_func)
            write_conditional_case(bc, block->get(i)->nestedContents, conditionIndex++);

    int switchFinishAddr = bc->opCount;
    
    // Update case-finish jumps
    Value unresolved;
    collect_unresolved(bc, s_conditional_done, &unresolved, startAddr);

    for (int i=0; i < unresolved.length(); i++) {
        int addr = unresolved.index(i)->field(s_addr)->as_s();
        bc->ops[addr].c = switchFinishAddr;
    }

    close_state_frame(bc, block, NULL);

    // Update liveness for term outputs
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL)
            break;
        Term* output = get_output_term(block->owningTerm, i);
        append_liveness(bc, output, firstOutputSlot + i);
    }

    comment(bc, "conditional fin");
}

void func_call(Bytecode* bc, Term* term)
{
    comment(bc, "func call");

    int inputCount = term->numInputs() - 1;
    int top = reserve_new_frame_slots(bc, inputCount);

    for (int i=0; i < inputCount; i++) {
        int inputSlot = top + 1 + i;
        load_term(bc, term->input(i + 1), inputSlot);
    }

    load_term(bc, term->input(0), top);
    append_op(bc, op_func_call_d, top, inputCount);
    append_liveness(bc, term, top);
}

void func_apply(Bytecode* bc, Term* term)
{
    comment(bc, "func apply");

    int top = reserve_new_frame_slots(bc, 1);

    load_term(bc, term->input(0), top);
    load_term(bc, term->input(1), top + 1);
    append_op(bc, op_func_apply_d, top, 0);
    append_liveness(bc, term, top);
}

void closure_value(Bytecode* bc, Term* term)
{
    comment(bc, "closure value");

    Block* block = term->nestedContents;

    int blockSlot = reserve_slots(bc, 1);
    int bindingsSlot;

    // Wrap all nonlocal values into a list.
    {
        int inputCount = count_closure_upvalues(block);
        int firstSlot = reserve_slots(bc, std::max(1, inputCount));
        int firstTermIndex = find_first_closure_upvalue(block);

        for (int i=0; i < inputCount; i++) {
            Term* term = block->get(i + firstTermIndex);
            ca_assert(term->function == FUNCS.upvalue);
            load_term(bc, term->input(0), firstSlot + i);
        }

        append_op(bc, op_make_list, firstSlot, inputCount);

        bindingsSlot = firstSlot;
    }

    set_block(load_const(bc, blockSlot), block);

    append_op(bc, op_make_func, blockSlot, bindingsSlot);
    append_liveness(bc, term, blockSlot);
}

bool exit_will_pass_minor_block(Block* block, Symbol exitType)
{
    if (is_switch_block(block) || is_case_block(block))
        return true;

    if (is_loop(block))
        return exitType == s_return;

    return false;
}

void pop_frames_for_early_exit(Bytecode* bc, Symbol exitType, Term* atTerm, Block* untilBlock)
{
    Block* block = atTerm->owningBlock;
    bool discard = exitType == s_discard;

    while (block != untilBlock) {

        if (should_write_state_header(bc, block)) {

            if (discard)
                append_op(bc, op_pop_discard_state_frame);
            else
                close_state_frame(bc, block, atTerm);

            if (is_loop(block) && should_write_state_header(bc, block)) {
                // Loop blocks have two state frames.
                if (discard)
                    append_op(bc, op_pop_discard_state_frame);
                else
                    append_op(bc, op_pop_state_frame);
            }
        }

        block = get_parent_block(block);
    }
}

void write_break(Bytecode* bc, Term* term)
{
    comment(bc, "break");
    Block* loop = find_enclosing_for_loop_contents(term);
    pop_frames_for_early_exit(bc, s_break, term, loop);

    close_state_frame(bc, loop, term);
    loop_move_locals_back_to_starting_slots(bc, loop, term);

    int addr = append_op(bc, op_jump);
    append_unresolved(bc, addr, s_break);
}

void write_continue(Bytecode* bc, Term* term)
{
    comment(bc, "continue");
    Block* loop = find_enclosing_for_loop_contents(term);
    pop_frames_for_early_exit(bc, s_continue, term, loop);

    close_state_frame(bc, loop, term);
    loop_advance_iterator(bc, loop);
    loop_move_locals_back_to_starting_slots(bc, loop, term);
    loop_preserve_iteration_result(bc, loop);

    int addr = append_op(bc, op_jump);
    append_unresolved(bc, addr, s_continue);
}

void write_discard(Bytecode* bc, Term* term)
{
    comment(bc, "discard");
    Block* loop = find_enclosing_for_loop_contents(term);
    pop_frames_for_early_exit(bc, s_discard, term, loop);

    if (should_write_state_header(bc, loop))
        append_op(bc, op_pop_discard_state_frame);
    loop_advance_iterator(bc, loop);
    loop_move_locals_back_to_starting_slots(bc, loop, term);

    int addr = append_op(bc, op_jump);
    append_unresolved(bc, addr, s_continue);
}

void write_return(Bytecode* bc, Term* term)
{
    comment(bc, "return");
    Block* majorBlock = find_enclosing_major_block(term);
    pop_frames_for_early_exit(bc, s_break, term, majorBlock);
    close_state_frame(bc, majorBlock, term);
    set_subroutine_output(bc, majorBlock, term->input(0));
    append_op(bc, op_ret_or_stop);
}

void write_declared_state(Bytecode* bc, Term* term)
{
    comment(bc, "declared state");

    int name_slot = reserve_slots(bc, 1);
    copy(term_name(term), load_const(bc, name_slot));

    int top = reserve_new_frame_slots(bc, 3);
    append_op(bc, op_get_state_value, name_slot, top+1);
    load_term(bc, term->input(1), top + 2);
    load_term(bc, term->input(2), top + 3);

    call(bc, top, 4, FUNCS.declared_state->nestedContents);
    append_liveness(bc, term, top);
}

void save_declared_state(Bytecode* bc, Block* block, Term* atTerm)
{
    if (bc->vm->noSaveState)
        return;

    Value location;
    if (atTerm != NULL)
        set_term_ref(&location, atTerm);
    else
        set_block(&location, block);

    for (int i=0; i < block->length(); i++) {
        Term* term = block->get(i);
        if (term->function != FUNCS.declared_state)
            continue;

        Term* result = find_name_at(&location, term_name(term));

        int slot = reserve_slots(bc, 2);
        copy(term_name(term), load_const(bc, slot));
        load_term(bc, result, slot+1);

        append_op(bc, op_save_state_value, slot, slot+1);
    }
}

void close_state_frame(Bytecode* bc, Block* block, Term* atTerm)
{
    if (should_write_state_header(bc, block)) {
        save_declared_state(bc, block, atTerm);
        append_op(bc, op_pop_state_frame);
    }
}

static bool block_contains_literal_symbol(Block* block, Symbol symbol)
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

bool block_needs_no_evaluation(Bytecode* bc, Block* block)
{
    if (bc->vm->noEffect && block_contains_literal_symbol(block, s_effect))
        return true;

    return false;
}

void write_term(Bytecode* bc, Term* term)
{
    bool ignore = is_value(term)
        || term_needs_no_evaluation2(term)
        || term->function == FUNCS.upvalue
        || term->function == FUNCS.loop_iterator;

    if (ignore)
        return;

    append_metadata(bc, mop_term_eval_start, 0)->term = term;

    if (use_inline_bytecode(bc, term))
        ;

    else if (term->function == FUNCS.dynamic_method)
        dynamic_method(bc, term);

    else if (term->function == FUNCS.while_loop || term->function == FUNCS.for_func)
        write_loop(bc, term->nestedContents);

    else if (term->function == FUNCS.loop_condition_bool)
        loop_condition_bool(bc, term);

    //else if (term->function == FUNCS.case_condition_bool)
    //    case_condition_bool(bc, term);

    else if (term->function == FUNCS.switch_func || term->function == FUNCS.if_block)
        write_conditional_chain(bc, term->nestedContents);

    else if (term->function == FUNCS.func_call || term->function == FUNCS.func_call_method)
        func_call(bc, term);

    else if (term->function == FUNCS.func_apply || term->function == FUNCS.func_apply_method)
        func_apply(bc, term);

    else if (term->function == FUNCS.closure_block || term->function == FUNCS.function_decl)
        closure_value(bc, term);

    else if (term->function == FUNCS.return_func)
        write_return(bc, term);
        
    else if (term->function == FUNCS.break_func)
        write_break(bc, term);

    else if (term->function == FUNCS.continue_func)
        write_continue(bc, term);

    else if (term->function == FUNCS.discard)
        write_discard(bc, term);

    else if (term->function == FUNCS.declared_state)
        write_declared_state(bc, term);

    else if (term->function == FUNCS.unknown_function_prelude)
        internal_error("bytecode: found unlinked call to unknown_function_prelude()");

    else
        write_normal_call(bc, term);

    append_metadata(bc, mop_term_eval_end, 0)->related_maddr =
        mop_find_active_mopcode(bc, mop_term_eval_start, -1);
}


void handle_function_inputs(Bytecode* bc, Block* block)
{
    // always reserve a slot for output
    bc->slotCount = 1;

    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            break;

        int inputSlot = 1 + i;
        bc->slotCount = inputSlot + 1;

        bool castNecessary = declared_type(placeholder) != TYPES.any;

        if (castNecessary)
            cast_fixed_type(bc, inputSlot, declared_type(placeholder));

        append_liveness(bc, placeholder, inputSlot);

        if (placeholder->boolProp(s_Multiple, false)) {
            // Collapse remaining args into a list.
            append_op(bc, op_varargs_to_list, i);
            break;
        }
    }

    int upvalueCount = count_closure_upvalues(block);
    if (upvalueCount > 0) {
        int firstSlot = bc->slotCount;
        bc->slotCount += upvalueCount;
        append_op(bc, op_splat_upvalues, firstSlot, upvalueCount);

        int firstUpvalue = find_first_closure_upvalue(block);
        for (int i=0; i < upvalueCount; i++) {
            Term* term = block->get(firstUpvalue + i);
            append_liveness(bc, term, firstSlot + i);
        }
    }
}

void major_block_contents(Bytecode* bc, Block* block)
{
    if (block_needs_no_evaluation(bc, block)) {
        comment(bc, "this block needs no evaluation");
        append_op(bc, op_ret_or_stop);
        return;
    }

    handle_function_inputs(bc, block);

    if (should_write_state_header(bc, block))
        write_state_header(bc, -1);

    // Function body
    int nativePatchIndex = find_native_func_index(block->world, block);
    if (nativePatchIndex != -1) {

        // Native func
        append_op(bc, op_native, nativePatchIndex);
        append_op(bc, op_ret_or_stop);

    } else {

        // Subroutine

        for (int i=0; i < block->length(); i++) {
            write_term(bc, block->get(i));

            if (block->get(i)->function == FUNCS.return_func)
                return;
        }

        close_state_frame(bc, block, NULL);
        set_subroutine_output(bc, block, get_output_placeholder(block, 0)->input(0));
        append_op(bc, op_ret_or_stop);
    }
}

void set_subroutine_output(Bytecode* bc, Block* block, Term* result)
{
    Term* placeholder = get_output_placeholder(block, 0);

    if (result == NULL) {
        append_op(bc, op_set_null, 0);
    } else {

        bool castNecessary = (declared_type(placeholder) != declared_type(result))
            && (declared_type(placeholder) != TYPES.any);

        load_term(bc, result, 0);

        if (castNecessary)
            cast_fixed_type(bc, 0, declared_type(placeholder));
    }
}

Bytecode* compile_major_block(Block* block, VM* vm)
{
    Bytecode* bc = new_bytecode(vm);

    append_metadata(bc, mop_major_block_start, 0)->block = block;

    #if DEBUG
        Value str;
        str.set_string("Block");
        if (block->owningTerm != NULL) {
            string_append(&str, ": ");
            string_append(&str, term_name(block->owningTerm));
        }
        comment(bc, &str);
    #endif

    int growFrameAddr = append_op(bc, op_grow_frame, 0);

    major_block_contents(bc, block);

    bc->ops[growFrameAddr].a = bc->slotCount;

    #if DEBUG
        comment(bc, "block fin");
    #endif

    end_major_frame(bc);

    return bc;
}

void relocate(Op* op, int constDelta, int opsDelta)
{
    switch (op->opcode) {
    case op_uncompiled_call:
    case op_dyn_method:
        op->c += constDelta;
        break;
    case op_call:
        op->c += opsDelta;
        break;
    case op_func_call_d:
    case op_func_apply_d:
        break;
    case op_jump:
    case op_jif:
    case op_jnif:
    case op_jeq:
    case op_jneq:
    case op_jgt:
    case op_jgte:
    case op_jlt:
    case op_jlte:
        op->c += opsDelta;
        break;
    case op_grow_frame:
        break;
    case op_load_const:
        op->a += constDelta;
        break;
    case op_load_i:
        break;
    case op_copy:
    case op_move:
    case op_set_null:
        break;
    case op_cast_fixed_type:
        op->b += constDelta;
        break;
    case op_make_func:
        break;
    case op_make_list:
        break;
    case op_push_state_frame:
    case op_pop_state_frame:
        break;
    case op_comment:
        op->a += constDelta;
        break;
    default:
        break;
    }
}

void dump_op(Bytecode* bc, Op op)
{
    switch (op.opcode) {
    case op_nope:
        printf("nope\n");
        break;
    case op_uncompiled_call:
        printf("uncompiled_call top:%d inputCount:%d const:%d\n", op.a, op.b , op.c);
        break;
    case op_call:
        printf("call top:%d inputCount:%d addr:%d\n", op.a, op.b , op.c);
        break;
    case op_func_call_d:
        printf("func_call_d top:%d inputCount:%d funcSlot:%d\n", op.a, op.b, op.c);
        break;
    case op_func_apply_d:
        printf("func_apply_d top:%d funcSlot:%d\n", op.a, op.c);
        break;
    case op_dyn_method:
        printf("dyn_method top:%d inputCount:%d nameLocationConst:%d\n", op.a, op.b, op.c);
        break;
    case op_jump: printf("jump %d\n", op.c); break;
    case op_jif: printf("jif %d %d\n", op.a, op.c); break;
    case op_jnif: printf("jnif %d %d\n", op.a, op.c); break;
    case op_jeq: printf("jeq %d %d %d\n", op.a, op.b, op.c); break;
    case op_jneq: printf("jneq %d %d %d\n", op.a, op.b, op.c); break;
    case op_jgt: printf("jgt %d %d %d\n", op.a, op.b, op.c); break;
    case op_jgte: printf("jgte %d %d %d\n", op.a, op.b, op.c); break;
    case op_jlt: printf("jlt %d %d %d\n", op.a, op.b, op.c); break;
    case op_jlte: printf("jlte %d %d %d\n", op.a, op.b, op.c); break;
    case op_grow_frame: printf("grow_frame %d\n", op.a); break;
    case op_load_const: printf("load_const const:%d slot:%d\n", op.a, op.b); break;
    case op_load_i: printf("load_i value:%d slot:%d\n", op.a, op.b); break;
    case op_native: printf("native %d\n", op.a); break;
    case op_ret: printf("ret\n"); break;
    case op_ret_or_stop: printf("ret_or_stop\n"); break;
    case op_varargs_to_list: printf("varargs_to_list firstSlot:%d\n", op.a); break;
    case op_splat_upvalues: printf("splat_upvalues firstSlot:%d count:%d\n", op.a, op.b); break;
    case op_copy: printf("copy fromSlot:%d toSlot:%d\n", op.a, op.b); break;
    case op_move: printf("move fromSlot:%d toSlot:%d\n", op.a, op.b); break;
    case op_set_null: printf("set_null slot:%d\n", op.a); break;
    case op_cast_fixed_type: printf("cast_fixed_type slot:%d\n", op.a); break;
    case op_make_list: printf("make_list first:%d count:%d\n", op.a, op.b); break;
    case op_make_func: printf("make_func %d %d\n", op.a, op.b); break;
    case op_add_i: printf("add_i left:%d right:%d dest:%d\n", op.a, op.b, op.c); break;
    case op_sub_i: printf("sub_i left:%d right:%d dest:%d\n", op.a, op.b, op.c); break;
    case op_mult_i: printf("mult_i left:%d right:%d dest:%d\n", op.a, op.b, op.c); break;
    case op_div_i: printf("div_i left:%d right:%d dest:%d\n", op.a, op.b, op.c); break;
    case op_push_state_frame: printf("push_state_frame %d %d\n", op.a, op.b); break;
    case op_pop_state_frame: printf("pop_state_frame\n"); break;
    case op_comment: {
        Value* val = (op.a < bc->consts.size) ? bc->consts[op.a] : NULL;
        if (val == NULL) {
            printf("# <bad const index>\n");
        } else if (is_string(val)) {
            printf("# %s\n", as_cstring(val));
        } else if (is_block(val)) {
            Block* block = as_block(val);
            Term* owningTerm = block->owningTerm;
            const char* blockName = "";
            if (owningTerm != NULL && !has_empty_name(owningTerm))
                blockName = as_cstring(term_name(owningTerm));
            printf("# Block#%d (%s)\n", block->id, blockName);
        }
        break;
    }
    default:
        printf("unrecognized by dump_op! 0x%x\n", op.opcode);
        break;
    }
}

void dump_mop(Bytecode*, BytecodeMetadata mop)
{
    printf("[addr:%d] ", mop.addr);
    switch (mop.mopcode) {
    case mop_term_eval_start: printf("term_eval_start"); break;
    case mop_term_eval_end: printf("term_eval_end"); break;
    case mop_term_live: printf("term_live"); break;
    case mop_major_block_start: printf("major_block_start"); break;
    case mop_major_block_end: printf("major_block_end"); break;
    }

    printf(" slot:%d", mop.slot);
    if (mop.term != NULL)
        printf(" term:#%d", mop.term->id);
    if (mop.block != NULL)
        printf(" block:#%d", mop.block->id);
    printf("\n");
}

void Bytecode::dump()
{
    Bytecode* bc = this;

    char* s;
    printf("[Bytecode dump]\n");

    printf(" Slot count = %d\n", bc->slotCount);
    
    printf(" [Ops (count = %d)]\n", bc->opCount);
    for (int i=0; i < bc->opCount; i++) {
        printf("  ");
        dump_op(bc, bc->ops[i]);
    }
}

void Bytecode::dump_metadata()
{
    Bytecode* bc = this;
    printf("[Bytecode metadata]\n");
    for (int i=0; i < bc->metadataSize; i++) {
        printf("  ");
        dump_mop(bc, bc->metadata[i]);
    }
}

int append_compiled_major_block(Bytecode* assembled, Block* block)
{
    Bytecode* bc = compile_major_block(block, assembled->vm);
    //bc->dump();

    int baseSlot = assembled->slotCount;
    assembled->slotCount += bc->slotCount;

    int baseConst = assembled->consts.size;
    int baseOp = assembled->opCount;

    grow_ops(assembled, baseOp + bc->opCount);
    memcpy(assembled->ops + baseOp, bc->ops, sizeof(Op) * bc->opCount);
    for (int i=baseOp; i < baseOp + bc->opCount; i++)
        relocate(&assembled->ops[i], baseConst, baseOp);

    assembled->consts.growBy(bc->consts.size);
    for (int i=0; i < bc->consts.size; i++)
        copy(bc->consts[i], assembled->consts[baseConst + i]);

    int baseMetadata = assembled->metadataSize;
    grow_metadata(assembled, baseMetadata + bc->metadataSize);
    memcpy(assembled->metadata + baseMetadata, bc->metadata,
        sizeof(BytecodeMetadata) * bc->metadataSize);

    for (int i=baseMetadata; i < baseMetadata + bc->metadataSize; i++) {
        assembled->metadata[i].addr += baseOp;
        if (mopcode_uses_related_maddr(assembled->metadata[i].mopcode))
            assembled->metadata[i].related_maddr += baseMetadata;
    }

    Value blockVal;
    set_block(&blockVal, block);
    assembled->blockToAddr.insert_val(&blockVal)->set_int(baseOp);

    free_bytecode(bc);

    return baseOp;
}

int find_compiled_major_block(Bytecode* bc, Block* block)
{
    Value blockVal;
    set_block(&blockVal, block);

    Value* found = bc->blockToAddr.val_key(&blockVal);

    if (found != NULL)
        return found->as_i();

    return -1;
}

int find_or_compile_major_block(Bytecode* bc, Block* block)
{
    Value blockVal;
    set_block(&blockVal, block);

    Value* found = bc->blockToAddr.val_key(&blockVal);

    if (found != NULL)
        return found->as_i();
    else
        return append_compiled_major_block(bc, block);
}

void vm_prepare_bytecode(VM* vm, VM* callingVM)
{
    vm_prepare_env(vm, callingVM);

    if (vm_check_if_hacks_changed(vm)) {
        vm_on_code_change(vm);
        vm_update_derived_hack_info(vm);
    }

    if (vm->bc == NULL) {
        vm->bc = new_bytecode(vm);
        append_compiled_major_block(vm->bc, vm->mainBlock);
    }
}

static inline Value* get_slot(VM* vm, int slot)
{
    int index = vm->stackTop + slot;
    return vm->stack[index];
}

static inline Value* get_const(VM* vm, int constIndex)
{
    return vm->bc->consts[constIndex];
}

static inline void do_call_op(VM* vm, int top, int inputCount, int toAddr)
{
    int prevTop = vm->stackTop;
    vm->stackTop = vm->stackTop + top;
    vm->inputCount = inputCount;
    vm->stack[vm->stackTop - 1]->set_int(prevTop);
    vm->stack[vm->stackTop - 2]->set_int(vm->pc);
    vm->pc = toAddr;
}

void vm_run(VM* vm, VM* callingVM)
{
    vm_prepare_bytecode(vm, callingVM);

    vm->pc = 0;
    vm->stackTop = 0;
    vm->stateTop = -1;
    vm->error = false;
    vm->inputCount = 0;
    vm_grow_stack(vm, 1);

    copy(&vm->topLevelUpvalues, &vm->incomingUpvalues);

    Op* ops = vm->bc->ops;

    #if TRACE_EXECUTION
        int executionDepth = 0;
    #endif

    #define trace_execution_indent() for (int i=0; i < executionDepth; i++) printf(" ");

    while (true) {

        ca_assert(vm->pc < vm->bc->opCount);

        Op op = ops[vm->pc++];

        #if TRACE_EXECUTION
            trace_execution_indent();
            printf("[pc:%d top:%d]: ", vm->pc-1, vm->stackTop);
            dump_op(vm->bc, op);
        #endif

        switch (op.opcode) {

        case op_nope:
            continue;
        case op_uncompiled_call: {
            vm->pc--;
            Block* block = get_const(vm, op.c)->asBlock();
            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;
            ops[vm->pc].opcode = op_call;
            ops[vm->pc].c = addr;
            continue;
        }
        case op_call: {
            do_call_op(vm, op.a, op.b, op.c);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_func_call_d: {
            Value* func = get_slot(vm, op.a);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("dyn_call with: %s\n", func->to_c_string());
            #endif

            Block* block = func_block(func);

            copy(func_bindings(func), &vm->incomingUpvalues);

            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;

            do_call_op(vm, op.a, op.b, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_func_apply_d: {
            Value* func = get_slot(vm, op.a);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("dyn_apply with: %s\n", func->to_c_string());
            #endif

            Block* block = func_block(func);
            copy(func_bindings(func), &vm->incomingUpvalues);

            int addr = find_or_compile_major_block(vm->bc, block);
            ops = vm->bc->ops;

            Value list;
            move(vm->stack[vm->stackTop + op.a + 1], &list);
            int inputCount = list.length();

            vm_grow_stack(vm, vm->stackTop + op.a + inputCount + 1);

            for (int i=0; i < inputCount; i++)
                copy(list.index(i), vm->stack[vm->stackTop + op.a + 1 + i]);

            do_call_op(vm, op.a, inputCount, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_dyn_method: {
            // grow in case we need to convert to Map.get call.
            vm_grow_stack(vm, op.a + 3);

            Value* object = get_slot(vm, op.a + 1);
            Value* nameLocation = get_const(vm, op.c);
            Block* method = find_method_on_type(get_value_type(object), nameLocation);

            if (method == NULL) {
                if (is_module_ref(object)) {
                    Term* function = module_lookup(vm->world, object, nameLocation->index(0));
                    if (function != NULL && is_function(function)) {
                        method = function->nestedContents;

                        // Before calling, we need to discard input 0 (the module ref).
                        int newTop = op.a;
                        int inputCount = op.b;
                        for (int input=1; input < inputCount; input++)
                            move(get_slot(vm, newTop+1+input), get_slot(vm, newTop+input));

                        int addr = find_or_compile_major_block(vm->bc, method);
                        ops = vm->bc->ops;
                        do_call_op(vm, op.a, inputCount - 1, addr);

                        #if TRACE_EXECUTION
                            executionDepth++;
                            trace_execution_indent();
                            printf("top is now: %d\n", vm->stackTop);
                        #endif

                        continue;
                    }
                } else if (is_hashtable(object)) {
                    Block* function = FUNCS.map_get->nestedContents;
                    int addr = find_or_compile_major_block(vm->bc, function);
                    ops = vm->bc->ops;

                    // Copy the name to input 1
                    Value* input1 = get_slot(vm, op.a + 2);
                    copy(nameLocation->index(0), input1);

                    // Temp, convert to symbol if needed
                    if (is_string(input1))
                        set_symbol(input1, symbol_from_string(as_cstring(input1)));

                    do_call_op(vm, op.a, 2, addr);
                    #if TRACE_EXECUTION
                        executionDepth++;
                        trace_execution_indent();
                        printf("top is now: %d\n", vm->stackTop);
                    #endif
                    continue;
                }

                Value msg;
                set_string(&msg, "Method ");
                string_append(&msg, nameLocation);
                string_append(&msg, " not found on ");
                string_append(&msg, object);
                return vm->throw_error(&msg);
            }
            
            int addr = find_or_compile_major_block(vm->bc, method);
            ops = vm->bc->ops;

            do_call_op(vm, op.a, op.b, addr);

            #if TRACE_EXECUTION
                executionDepth++;
                trace_execution_indent();
                printf("top is now: %d\n", vm->stackTop);
            #endif

            continue;
        }
        case op_jump:
            vm->pc = op.c;
            continue;
        case op_jif: {
            Value* a = get_slot(vm, op.a);
            if (a->asBool())
                vm->pc = op.c;
            continue;
        }
        case op_jnif: {
            Value* a = get_slot(vm, op.a);
            if (!a->asBool())
                vm->pc = op.c;
            continue;
        }
        case op_jeq: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            if (equals(a, b))
                vm->pc = op.c;
            continue;
        }
        case op_jneq: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            if (!equals(a, b))
                vm->pc = op.c;
            continue;
        }
        case op_grow_frame: {
            vm_grow_stack(vm, vm->stackTop + op.a);
            continue;
        }
        case op_load_const: {
            Value* val = get_const(vm, op.a);
            Value* slot = get_slot(vm, op.b);
            copy(val, slot);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("loaded const %s to s%d\n", slot->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_load_i: {
            Value* slot = get_slot(vm, op.b);
            set_int(slot, op.a);
            continue;
        }
        case op_native: {
            EvaluateFunc func = get_native_func(vm->world, op.a);
            func(vm);

            // some funcs can compile bytecode which invalidates our pointers.
            ops = vm->bc->ops;

            if (vm->error)
                return;

            continue;
        }
        case op_ret_or_stop: {
            if (vm->stackTop == 0) {
                vm_cleanup_on_stop(vm);
                return;
            }
            // fallthrough
        }
        case op_ret: {
            #if DEBUG
                int prevTop = vm->stackTop;
            #endif
            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("return value is: %s\n", get_slot(vm, 0)->to_c_string());
                executionDepth--;
            #endif

            vm->pc = get_slot(vm, -2)->as_i();
            vm->stackTop = get_slot(vm, -1)->as_i();

            continue;
        }
        case op_varargs_to_list: {
            Value list;
            int firstInputIndex = op.a;
            int inputCount = vm->inputCount - firstInputIndex;
            ca_assert(inputCount >= 0);
            list.set_list(inputCount);
            for (int i=0; i < inputCount; i++)
                copy(vm->stack[vm->stackTop + 1 + firstInputIndex + i], list.index(i));
            move(&list, vm->stack[vm->stackTop + firstInputIndex + 1]);
            continue;
        }
        case op_splat_upvalues: {
            if (is_null(&vm->incomingUpvalues))
                return vm->throw_str("internal error: Called a closure without upvalues list");

            for (int i=0; i < op.b; i++) {
                Value* value = vm->incomingUpvalues.index(i);
                copy(value, get_slot(vm, op.a + i));
            }
            set_null(&vm->incomingUpvalues);
            continue;
        }
        case op_copy: {
            Value* left = get_slot(vm, op.a);
            Value* right = get_slot(vm, op.b);
            copy(left, right);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("copied value: %s to s%d\n", right->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_move: {
            Value* left = get_slot(vm, op.a);
            Value* right = get_slot(vm, op.b);
            move(left, right);

            #if TRACE_EXECUTION
                trace_execution_indent();
                printf("moved value: %s to s%d\n", right->to_c_string(), op.b + vm->stackTop);
            #endif

            continue;
        }
        case op_set_null: {
            Value* val = get_slot(vm, op.a);
            set_null(val);
            continue;
        }
        case op_cast_fixed_type: {
            Value* val = get_slot(vm, op.a);
            Type* type = as_type(get_const(vm, op.b));
            bool success = cast(val, type);
            if (!success) {
                Value msg;
                set_string(&msg, "Couldn't cast ");
                string_append(&msg, val);
                string_append(&msg, " to type ");
                string_append(&msg, &type->name);
                return vm->throw_error(&msg);
            }
            continue;
        }
        case op_make_list: {
            int count = op.b;

            Value list;
            list.set_list(count);

            for (int i=0; i < count; i++)
                copy(get_slot(vm, op.a + i), list.index(i));

            move(&list, get_slot(vm, op.a));
            continue;
        }
        case op_make_func: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);

            Value func;
            set_closure(a, a->asBlock(), b);
            continue;
        }
        case op_add_i: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            Value* c = get_slot(vm, op.c);
            set_int(c, a->as_i() + b->as_i());
            continue;
        }
        case op_sub_i: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            Value* c = get_slot(vm, op.c);
            set_int(c, a->as_i() - b->as_i());
            continue;
        }
        case op_mult_i: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            Value* c = get_slot(vm, op.c);
            set_int(c, a->as_i() * b->as_i());
            continue;
        }
        case op_div_i: {
            Value* a = get_slot(vm, op.a);
            Value* b = get_slot(vm, op.b);
            Value* c = get_slot(vm, op.c);
            set_int(c, a->as_i() / b->as_i());
            continue;
        }
        case op_push_state_frame:
            if (op.b == ((u16) -1)) {
                Value* key = NULL;
                Term* caller = vm_calling_term(vm);
                if (caller != NULL)
                    key = unique_name(caller);
                push_state_frame(vm, vm->stackTop + op.a, key);
            } else {
                push_state_frame(vm, vm->stackTop + op.a, get_slot(vm, op.b));
            }

            continue;
        case op_pop_state_frame:
            pop_state_frame(vm);
            continue;
        case op_pop_discard_state_frame:
            pop_discard_state_frame(vm);
            continue;
        case op_get_state_value:
            get_state_value(vm, get_slot(vm, op.a), get_slot(vm, op.b));
            continue;
        case op_save_state_value:
            save_state_value(vm, get_slot(vm, op.a), get_slot(vm, op.b));
            continue;
        case op_comment:
            continue;
        default: {
            printf("unrecognized op: 0x%x\n", op.opcode);
            internal_error("unrecognized op in vm_run");
        }
        }
    }
}

void exec(Block* block)
{
    VM* vm = new_vm(block);
    vm_run(vm, NULL);
}

int find_metadata_addr_for_addr(Bytecode* bc, int addr, int startRange, int endRange)
{
    if ((startRange + 1) >= endRange)
        return startRange;

    int guess = (startRange + endRange) / 2;

    ca_assert(guess >= 0 && guess < bc->metadataSize);

    BytecodeMetadata* md = &bc->metadata[guess];

    if (md->addr == addr) {
        return guess;
    } else if (addr < md->addr)
        return find_metadata_addr_for_addr(bc, addr, startRange, guess);
    else
        return find_metadata_addr_for_addr(bc, addr, guess, endRange);

    return 0; // unreachable
}

int find_metadata_addr_for_addr(Bytecode* bc, int addr)
{
    int maddr = find_metadata_addr_for_addr(bc, addr, 0, bc->metadataSize);

    // If we didn't find an exact match, find the next lowest addr.
    while (maddr > 0 && bc->metadata[maddr].addr > addr)
        maddr--;

    // Pick the *last* metadata with this addr (there might be multiple)
    while (maddr > 0 && bc->metadata[maddr+1].addr == addr)
        maddr++;

    return maddr;
}

int mop_find_active_mopcode(Bytecode* bc, int mopcode, int maddr)
{
    bool stayWithinMinorFrame = false; //mopcode == mop_state_header;

    if (maddr == -1)
        maddr = bc->metadataSize - 1;

    for (; maddr >= 0; maddr--) {

        if (bc->metadata[maddr].mopcode == mopcode)
            return maddr;
        if (bc->metadata[maddr].mopcode == mop_major_block_start)
            return -1;
    }
    return -1;
}

bool mopcode_uses_related_maddr(int mopcode)
{
    return mopcode == mop_term_eval_end
        //|| mopcode == mop_minor_block_end
        || mopcode == mop_major_block_end;
}

Term* find_active_term(Bytecode* bc, int addr)
{
    if (bc == NULL || bc->metadataSize == 0)
        return NULL;

    int maddr = find_metadata_addr_for_addr(bc, addr);

    for (; maddr >= 0; maddr--) {
        BytecodeMetadata md = bc->metadata[maddr];
        switch (md.mopcode) {
        case mop_term_eval_start:
            return md.term;
        case mop_term_eval_end:
        case mop_major_block_start:
            return NULL;
        }
    }
    return NULL;
}

Block* find_active_major_block(Bytecode* bc, int addr)
{
    // Finds the active block at 'addr', by walking backwards to find the nearest
    // mop_major_block_start.
    
    if (bc == NULL || bc->metadataSize == 0)
        return NULL;

    int maddr = find_metadata_addr_for_addr(bc, addr);
    maddr = mop_find_active_mopcode(bc, mop_major_block_start, maddr);
    ca_assert(maddr != -1);
    return bc->metadata[maddr].block;
}

} // namespace circa
