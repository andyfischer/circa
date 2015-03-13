// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "bytecode.h"
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

#define DUMP_COMPILED_BYTECODE 0
#define TRACE_OPTIMIZATIONS    0
#define TRACE_SLOT_COMPACTION  0

/*

Bytecode.cpp

# Glossary #

  major block:  A block that corresponds to a whole function or subroutine. 

  minor block: A block that corresponds to a switch block (including an if-else chain),
    or a loop

  slot: A stack-based value. Could also be called a "register". All opcodes operate on
    values in slots. Slot values are temporary and most are discard at the end of the
    VM's evaluation.

  pc: Program counter, an index/position in the op array.

  op: operation, a single VM "command"

  liveness: A record that consists of a term, a slot, and a PC start/stop range. Inside
    this range (when the slot is "live"), the slot's value is still a valid result for this
    term. Once the slot is reused for something else, or the frame is discarded, then the slot is
    "dead" for the term.

# Stack Frame Layout #

During a function call, the stack slots are:

       [-2 ]        - ret_pc, the addr to jump to on return
       [-1 ]        - ret_top, the stackTop to restore on return
top -> [ 0 ]        - output, the function's one output slot
       [1 ... N+1]  - N input slot(s), each located at (top + 1 + index)
       [N+1 ...]    - the function's temporaries

 */



namespace circa {
    
int op_flags(int opcode);
Liveness* get_liveness(Bytecode* bc, int slot);
Value* append_const(Bytecode* bc, int* index);
void comment(Bytecode* bc, Value* val);
void comment(Bytecode* bc, const char* str);
Liveness* set_term_live(Bytecode* bc, Term* term, int slot);
bool block_needs_no_evaluation(Bytecode* bc, Block* block);
Value* load_const(Bytecode* bc, int slot);
void write_term(Bytecode* bc, Term* term);
void write_normal_call(Bytecode* bc, Term* term);
int find_compiled_major_block(Bytecode* bc, Block* block);
void close_conditional_case(Bytecode* bc, Block* block);
void loop_advance_iterator(Bytecode* bc, Block* loop);
void loop_handle_locals_at_iteration_end(Bytecode* bc, Block* loop, Term* atTerm, Symbol continueOrBreak);
void loop_preserve_iteration_result(Bytecode* bc, Block* loop);
void save_declared_state(Bytecode* bc, Block* block, Term* atTerm);
void close_state_frame(Bytecode* bc, Block* block, Term* atTerm);
void set_subroutine_output(Bytecode* bc, Block* block, Term* result);

Bytecode* new_bytecode(VM* vm)
{
    Bytecode* bc = (Bytecode*) malloc(sizeof(Bytecode));
    initialize_null(&bc->unresolved);
    bc->topMinorFrame = NULL;
    bc->liveness = NULL;
    bc->livenessCount = 0;
    initialize_null(&bc->blockToAddr);
    set_list(&bc->unresolved, 0);
    set_hashtable(&bc->blockToAddr);
    bc->metadata = NULL;
    bc->metadataSize = 0;
    bc->slotCount = 0;
    bc->nextFreeSlot = 0;
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
    set_null(&bc->unresolved);
    free(bc->ops);
    free(bc->metadata);
}

int reserve_slots(Bytecode* bc, int count)
{
    int first = bc->slotCount;
    bc->slotCount += count;
    bc->nextFreeSlot += count;
    return first;
}

int reserve_new_frame_slots(Bytecode* bc, int inputCount)
{
    // Reserve slots for a function call. Returns the first slot (the function's output)
    return reserve_slots(bc, inputCount + 3) + 2;
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

void update_slot_first_write(Bytecode* bc, int slot)
{
    Liveness* liveness = get_liveness(bc, slot);
    if (liveness->writePc == -1)
        liveness->writePc = bc->opCount - 1;
}

void update_slot_last_read(Bytecode* bc, int slot)
{
    Liveness* liveness = get_liveness(bc, slot);

    if (liveness->writePc == -1)
        internal_error("update_slot_last_read called before slot had a write");

    liveness->lastReadPc = bc->opCount - 1;
}

struct OpReadSlotsIter {
    // Iterator that steps through each slot that is read by this operation.
    // Slot-reading operations come in two flavors:
    //   1) Ops that read one or more slot indexes in A, B, or C
    //   2) Ops that read N slots (where A = slot index 0, and B = slot count)
    
    Op* op;
    int opflags;
    int step;

    bool done() {
        if (opflags & OP_READS_N_SLOTS)
            return step >= op->b;
        else
            return step >= 3;
    }

    int current() {
        if (opflags & OP_READS_N_SLOTS)
            return op->a + step + 1;

        if (step == 0) return op->a;
        if (step == 1) return op->b;
        if (step == 2) return op->c;

        return -1;
    }

    void advance() {
        step++;
        settle();
    }

    void settle() {
        if (opflags & OP_READS_N_SLOTS)
            return;

        if (step == 0 && !(opflags & OP_READS_SLOT_A))
            step++;
        if (step == 1 && !(opflags & OP_READS_SLOT_B))
            step++;
        if (step == 2 && !(opflags & OP_READS_SLOT_C))
            step++;
    }
};

OpReadSlotsIter op_slots_read_iter(Op* op)
{
    OpReadSlotsIter iter;
    iter.op = op;
    iter.opflags = op_flags(op->opcode);
    iter.step = 0;
    iter.settle();
    return iter;
}

int append_op(Bytecode* bc, u8 opcode, u16 a = 0, u16 b = 0, u16 c = 0)
{
    int pc = bc->opCount;

    ca_assert(opcode != 0);

    grow_ops(bc, bc->opCount + 1);

    Op* op = &bc->ops[pc];
    op->opcode = opcode;
    op->a = a;
    op->b = b;
    op->c = c;

    // Update liveness
    int opflags = op_flags(opcode);

    if (opflags & OP_WRITES_SLOT_A)
        update_slot_first_write(bc, a);

    for (OpReadSlotsIter it = op_slots_read_iter(op); !it.done(); it.advance())
        update_slot_last_read(bc, it.current());

    return pc;
}

void update_op_reads_to_end_of_loop(Bytecode* bc, MinorFrame* loopFrame, int iterationStart)
{
    // Called when we finish a loop. When a slot that's outside the loop is
    // read by an operation that's inside the loop, we adjust the "last read"
    // pc of that slot to the end of the loop. Otherwise, the first iteration
    // might consume the value, when it needs to stay available for subsequent
    // iterations.

    for (int pc=iterationStart; pc < bc->opCount; pc++) {
        Op* op = &bc->ops[pc];

        for (OpReadSlotsIter iter = op_slots_read_iter(op); !iter.done(); iter.advance()) {
            if (iter.current() < loopFrame->loopFirstLocalInitial)
                update_slot_last_read(bc, iter.current());
        }
    }
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

MinorFrame* start_minor_frame(Bytecode* bc, Block* block)
{
    MinorFrame* minorFrame = (MinorFrame*) malloc(sizeof(MinorFrame));
    minorFrame->parent = bc->topMinorFrame;
    minorFrame->block = block;
    minorFrame->firstPc = bc->opCount;
    minorFrame->hasBeenExited = false;
    minorFrame->firstTemporarySlot = bc->nextFreeSlot;
    minorFrame->hasStateFrame = false;
    minorFrame->loopIteratorSlot = -1;
    minorFrame->loopProduceOutput = false;
    minorFrame->loopOutputSlot = -1;
    bc->topMinorFrame = minorFrame;
    return minorFrame;
}

void end_minor_frame(Bytecode* bc)
{
    // Pop the top MinorFrame. All local slot allocations are now dead.

    MinorFrame* minorFrame = bc->topMinorFrame;

    bc->topMinorFrame = minorFrame->parent;
    free(minorFrame);
}

MinorFrame* find_minor_frame(Bytecode* bc, Block* block)
{
    MinorFrame* minorFrame = bc->topMinorFrame;
    while (minorFrame != NULL) {
        if (minorFrame->block == block)
            return minorFrame;
        minorFrame = minorFrame->parent;
    }
    return NULL;
}

bool minor_block_was_exited(Bytecode* bc)
{
    return bc->topMinorFrame->hasBeenExited;
}

Liveness* get_liveness(Bytecode* bc, int slot)
{
    if (slot >= bc->livenessCount) {
        int newSize = slot + 1;
        bc->liveness = (Liveness*) realloc(bc->liveness, sizeof(Liveness) * newSize);
        for (int i=bc->livenessCount; i < newSize; i++) {
            Liveness* liveness = &bc->liveness[i];
            liveness->term = 0;
            liveness->writePc = -1;
            liveness->lastReadPc = -1;
            liveness->relocateable = true;
        }
        bc->livenessCount = newSize;
    }
    
    return &bc->liveness[slot];
}

Liveness* set_term_live(Bytecode* bc, Term* term, int slot)
{
    Liveness* liveness = get_liveness(bc, slot);
    liveness->term = term;
    update_slot_first_write(bc, slot);
    return liveness;
}

int find_live_slot(Bytecode* bc, Term* term, bool allowNull = false)
{
    for (int i=bc->livenessCount - 1; i >= 0; i--)
        if (bc->liveness[i].term == term)
            return i;

    if (allowNull)
        return -1;

    internal_error("find_live_slot: term is not live");
    return -1;
}

bool load_term_static(Bytecode* bc, Term* term, int slot)
{
    // Try to load a static value for this term to 'slot'. Returns true if successful.

    if (term == NULL) {
        append_op(bc, op_set_null, slot);
        return true;
    }

    Value* foundOverride = hashtable_get_term_key(&bc->vm->termOverrides, term);
    if (foundOverride != NULL) {
        load_const(bc, slot)->set(foundOverride);
        return true;
    }

    if (term->function == FUNCS.break_func
            || term->function == FUNCS.continue_func
            || term->function == FUNCS.discard
            || term->function == FUNCS.return_func) {
        // No output value
        append_op(bc, op_set_null, slot);
        return true;
    }

    if (has_static_value(term)) {
        int constIndex;
        copy(term_value(term), append_const(bc, &constIndex));
        append_op(bc, op_load_const, slot, constIndex);
        //set_term_live(bc, term, slot);
        return true;
    }

    return false;
}

bool load_term_live(Bytecode* bc, Term* term, int toSlot, bool move)
{
    int liveSlot = find_live_slot(bc, term, true);

    if (liveSlot == -1)
        return false;

    append_op(bc, move ? op_move : op_copy, toSlot, liveSlot);

    return true;
}

void load_term(Bytecode* bc, Term* term, int toSlot, bool move)
{
    if (load_term_static(bc, term, toSlot))
        return;

    if (load_term_live(bc, term, toSlot, move))
        return;

    internal_error("load_term: term is not loadable");
}

void load_input_term(Bytecode* bc, Term* term, Term* input, int toSlot)
{
    bool move = can_consume_term_result(input, term);

    if (move)
        comment(bc, "can move input");

    load_term(bc, input, toSlot, move);
}

int find_or_load_term(Bytecode* bc, Term* term)
{
    int liveSlot = find_live_slot(bc, term, true);

    if (liveSlot != -1)
        return liveSlot;

    int slot = reserve_slots(bc, 1);
    load_term(bc, term, slot, false);
    return slot;
}

void append_unresolved_jump(Bytecode* bc, int addr, Symbol type)
{
    bc->unresolved.append()->set_hashtable()
        ->set_field_int(s_addr, addr)
        ->set_field_sym(s_type, type);
}

void resolve_unresolved_jump(Bytecode* bc, Symbol ofType, int afterAddr, int jumpAddr)
{
    for (int i=0; i < bc->unresolved.length(); i++) {
        Value* unresolved = bc->unresolved.index(i);
        if (unresolved->field(s_type)->as_s() != ofType)
            continue;

        if (afterAddr > 0 && unresolved->field(s_addr)->as_i() < afterAddr)
            continue;

        int opAddr = unresolved->field(s_addr)->as_i();
        bc->ops[opAddr].c = jumpAddr;

        set_null(unresolved);
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
    append_op(bc, op_load_const, slot, constIndex);
    return value;
}

void cast_fixed_type(Bytecode* bc, int slot, Type* type)
{
    int constIndex;
    set_type(append_const(bc, &constIndex), type);
    append_op(bc, op_cast_fixed_type, slot, slot, constIndex);
}

bool should_write_state_header(Bytecode* bc, Block* block)
{
    // No state allowed in while-loop. Maybe temp, maybe permanant.
    if (is_while_loop(block))
        return false;
    return block_has_state(block) != s_no;
}

void write_state_header(Bytecode* bc, int keySlot)
{
    if (keySlot == -1)
        append_op(bc, op_push_state_frame);
    else
        append_op(bc, op_push_state_frame_dkey, keySlot);

    bc->topMinorFrame->hasStateFrame = true;
}

void write_state_header_with_term_name(Bytecode* bc, Term* term)
{
    int slot = reserve_slots(bc, 1);
    copy(unique_name(term), load_const(bc, slot));
    write_state_header(bc, slot);
}

void write_pop_state_frame(Bytecode* bc)
{
    if (bc->vm->noSaveState)
        append_op(bc, op_pop_discard_state_frame);
    else
        append_op(bc, op_pop_state_frame);
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
    load_input_term(bc, term, term->input(0), top+1);
    load_input_term(bc, term, term->input(1), top+2);

    append_op(bc, opcode, top, top+1, top+2);
    set_term_live(bc, term, top);
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
    append_op(bc, op_precall, top, inputCount);

    for (int i=0; i < inputCount; i++)
        load_input_term(bc, term, term->input(i), top + 1 + i);
    
    append_op(bc, op_dyn_method, top, inputCount, constIndex);
    set_term_live(bc, term, top);
}

void write_loop(Bytecode* bc, Block* loop)
{
    /*
    Notes on slot allocation inside loops:

    The loop has "local" slots, which are used for names that are rebound inside the loop.

    Pseudocode for the way that locals are handled:

        [outside loop, various "outer" values are live]

        Allocate local-output slots

        loop {
            [loop setup section]
            Local-initial slots are allocated
            Copy values from outers to local-initial slots
            Initialize iterator
            Initialize output list (if active)

            [Iteration start]
            [Done check]
            Check if iterator is done
              If so:
                Jump to [Move locals and finish]
              If not:
                Continue on to [Loop contents]

            [Loop contents]
            Evaluate inner terms
            Inside the contents, we might find a control flow term:
              (Continue and Discard)
                Copy active locals to local-initial slots
                Jump to [Iteration start]
              (Break)
                Copy active locals to local-output slots
                Jump to [Finish loop]

            Jump to [Iteration start]

            [Move locals and finish]
              (uses unresolved jump: s_done)
            Move local-initial slots to local-output 

            [Finish loop]
              (uses unresolved jump: s_break)
        }

        Local-output slots are now live
     */

    comment(bc, "loop setup");

    Term* callingTerm = loop->owningTerm;
    if (should_write_state_header(bc, loop))
        write_state_header_with_term_name(bc, callingTerm);
        
    bool produceOutput = term_is_observable(callingTerm);

    int outputSlot = -1;
    if (produceOutput) {
        outputSlot = reserve_new_frame_slots(bc, 1);
        append_op(bc, op_precall, outputSlot, 1);
        append_op(bc, op_load_i, outputSlot + 1, 0);
        call(bc, outputSlot, 1, FUNCS.blank_list->nestedContents);
    }

    int firstLocalOutput = reserve_slots(bc, count_input_placeholders(loop));
    int firstLocalInitial = reserve_slots(bc, count_input_placeholders(loop));

    comment(bc, "load values into local-initial");
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(loop, i);
        if (placeholder == NULL)
            break;

        int slot = firstLocalInitial + i;
        load_input_term(bc, placeholder, placeholder->input(0), slot);
        set_term_live(bc, placeholder, slot);
    }

    MinorFrame* minorFrame = start_minor_frame(bc, loop);
    minorFrame->loopProduceOutput = produceOutput;
    minorFrame->loopOutputSlot = outputSlot;
    minorFrame->loopFirstLocalInitial = firstLocalInitial;
    minorFrame->loopFirstLocalOutput = firstLocalOutput;

    if (is_for_loop(loop)) {
        comment(bc, "load initial iterator value");
        Term* iterator = loop_find_iterator(loop);
        int iteratorSlot = reserve_slots(bc, 1);
        load_input_term(bc, iterator, iterator->input(0), iteratorSlot);
        set_term_live(bc, iterator, iteratorSlot);
        minorFrame->loopIteratorSlot = iteratorSlot;
    }
    
    // Start loop
    int iterationStart = bc->opCount;
    comment(bc, "loop iteration start");

    Term* doneTerm = NULL;
    Term* keyTerm = NULL;
    if (is_for_loop(loop)) {
        doneTerm = loop_find_done_call(loop);
        keyTerm = loop_find_key(loop);
    }

    comment(bc, "loop contents");
    
    // Contents
    for (int i=0; i < loop->length(); i++) {
        if (minor_block_was_exited(bc))
            break;

        Term* term = loop->get(i);

        if (term == keyTerm) {
            comment(bc, "loop key");
            write_term(bc, keyTerm);
            if (should_write_state_header(bc, loop))
                write_state_header(bc, find_or_load_term(bc, keyTerm));
            continue;
        }

        if (term == doneTerm) {
            comment(bc, "done check");
            doneTerm = loop_find_done_call(loop);
            write_term(bc, doneTerm);
            int addr = append_op(bc, op_jif, find_live_slot(bc, doneTerm));
            append_unresolved_jump(bc, addr, s_done);
            continue;
        }

        write_term(bc, term);
    }

    // Prepare for next iteration.

    close_state_frame(bc, loop, NULL);
    loop_advance_iterator(bc, loop);
    loop_handle_locals_at_iteration_end(bc, loop, NULL, s_continue);
    loop_preserve_iteration_result(bc, loop);

    comment(bc, "jump back to loop start");
    append_op(bc, op_jump, 0, 0, iterationStart);

    // unresolved 'continue' jumps go to iterationStart
    resolve_unresolved_jump(bc, s_continue, iterationStart, iterationStart);

    // unresolved 'done' jumps go here
    resolve_unresolved_jump(bc, s_done, iterationStart, bc->opCount);

    comment(bc, "move locals and finish");
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(loop, i);
        if (placeholder == NULL)
            break;
        append_op(bc, op_move, firstLocalOutput + i, firstLocalInitial + i);
    }

    comment(bc, "finish loop");
    resolve_unresolved_jump(bc, s_break, iterationStart, bc->opCount);
    update_op_reads_to_end_of_loop(bc, minorFrame, iterationStart);

    if (should_write_state_header(bc, loop))
        write_pop_state_frame(bc);

    end_minor_frame(bc);

    // Update liveness
    if (produceOutput)
        set_term_live(bc, callingTerm, outputSlot);

    for (int i=0;; i++) {
        Term* termOutput = get_extra_output(callingTerm, i);
        if (termOutput == NULL)
            break;
        set_term_live(bc, termOutput, firstLocalOutput + i);
    }
}

void loop_advance_iterator(Bytecode* bc, Block* loop)
{
    MinorFrame* mframe = find_minor_frame(bc, loop);

    comment(bc, "loop iterator advance");
    Term* advanceTerm = loop_find_iterator_advance(loop);
    if (is_for_loop(loop)) {
        write_term(bc, advanceTerm);
        load_term(bc, advanceTerm, mframe->loopIteratorSlot, true);
    }
}

void loop_handle_locals_at_iteration_end(Bytecode* bc, Block* loop, Term* atTerm, Symbol continueOrBreak)
{
    ca_assert(continueOrBreak == s_continue || continueOrBreak == s_break);
    
    MinorFrame* mframe = find_minor_frame(bc, loop);

    comment(bc, "move looped values");

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

        int slot;
        if (continueOrBreak == s_continue)
            slot = mframe->loopFirstLocalInitial + i;
        else
            slot = mframe->loopFirstLocalOutput + i;

        load_term(bc, finalValue, slot, false);
    }
}

void loop_preserve_iteration_result(Bytecode* bc, Block* loop)
{
    MinorFrame* mframe = find_minor_frame(bc, loop);

    if (mframe->loopProduceOutput) {
        comment(bc, "preserve iteration result");
        Term* result = get_output_placeholder(loop, 0)->input(0);

        // TODO: Modify building to just connect the input_placeholder directly to this term.
        if (result->function == FUNCS.output)
            result = result->input(0);

        int top = reserve_new_frame_slots(bc, 2);
        append_op(bc, op_precall, top, 2);
        append_op(bc, op_copy, top + 1, mframe->loopOutputSlot);
        load_term(bc, result, top + 2, false);
        call(bc, top, 2, FUNCS.list_append->nestedContents);
        append_op(bc, op_copy, mframe->loopOutputSlot, top);
    }
}

void loop_condition_bool(Bytecode* bc, Term* term)
{
    comment(bc, "loop condition");
    int slot = reserve_slots(bc, 1);
    load_input_term(bc, term, term->input(0), slot);
    int addr = append_op(bc, op_jnif, slot);
    append_unresolved_jump(bc, addr, s_done);
}

void maybe_write_not_enough_inputs_error(Bytecode* bc, Block* func, int found)
{
    ca_assert(func != NULL);
    int expected = count_input_placeholders(func);
    int varargs = has_variable_args(func);

    if (varargs)
        expected--;

    if (found >= expected)
        return;

    Value message;
    set_string(&message, "Not enough inputs for function '");
    string_append(&message, func->name());
    string_append(&message, "': expected ");
    string_append(&message, expected);
    if (varargs)
        string_append(&message, " (or more)");
    string_append(&message, " and found ");
    string_append(&message, found);

    int top = reserve_new_frame_slots(bc, 1);
    append_op(bc, op_precall, top, 1);
    move(&message, load_const(bc, top + 1));
    call(bc, top, 1, FUNCS.error->nestedContents);
}

void maybe_write_too_many_inputs_error(Bytecode* bc, Block* func, int found)
{
    int expected = count_input_placeholders(func);
    int varargs = has_variable_args(func);

    if (varargs || found <= expected)
        return;

    Value message;
    set_string(&message, "Too many inputs for function '");
    string_append(&message, func->name());
    string_append(&message, "': expected ");
    string_append(&message, expected);
    string_append(&message, " and found ");
    string_append(&message, found);
    
    int top = reserve_new_frame_slots(bc, 1);
    append_op(bc, op_precall, top, 1);
    move(&message, load_const(bc, top + 1));
    call(bc, top, 1, FUNCS.error->nestedContents);
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
        comment(bc, "this block needs no evaluation");
        int top = reserve_new_frame_slots(bc, 0);
        append_op(bc, op_set_null, top);
        set_term_live(bc, term, top);
        return;
    }

    int inputCount = term->numInputs();

    maybe_write_not_enough_inputs_error(bc, target, inputCount);
    maybe_write_too_many_inputs_error(bc, target, inputCount);

    int top = reserve_new_frame_slots(bc, inputCount);

    append_op(bc, op_precall, top, inputCount);
    
    for (int i=0; i < inputCount; i++) {
        int inputSlot = top + 1 + i;
        load_input_term(bc, term, term->input(i), inputSlot);
    }

    if (count_closure_upvalues(target) != 0) {
        load_input_term(bc, term, term->function, top);
        append_op(bc, op_func_call_d, top, inputCount);
    } else {
        call(bc, top, inputCount, target);
    }

    set_term_live(bc, term, top);
}

void write_conditional_case(Bytecode* bc, Block* block, int conditionIndex)
{
    start_minor_frame(bc, block);

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
        load_term(bc, condition, conditionSlot, false);

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
        //set_term_live(bc, NULL, slot);
        write_state_header(bc, slot);
    }

    // Case contents
    for (int i=0; i < block->length(); i++) {
        if (minor_block_was_exited(bc))
            break;

        write_term(bc, block->get(i));
    }

    close_state_frame(bc, block, NULL);

    comment(bc, "case move outputs");

    Block* parentBlock = get_parent_block(block);
    MinorFrame* parentMframe = find_minor_frame(bc, parentBlock);

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

        load_term(bc, localResult, parentMframe->conditionalFirstOutputSlot + i, false);
    }

    comment(bc, "case fin");

    // Jump to switch-finish.
    int addr = append_op(bc, op_jump, 0, 0, 0);
    append_unresolved_jump(bc, addr, s_conditional_done);
    end_minor_frame(bc);
}

void write_conditional_chain(Bytecode* bc, Block* block)
{
    comment(bc, "conditional start");
    int startAddr = bc->opCount;
    start_minor_frame(bc, block);

    if (should_write_state_header(bc, block))
        write_state_header_with_term_name(bc, block->owningTerm);

    // Assign output slots. Each conditional block will put its respective outputs here.
    int outputCount = count_output_placeholders(block);
    int firstOutputSlot = reserve_slots(bc, outputCount);
    bc->topMinorFrame->conditionalFirstOutputSlot = firstOutputSlot;

    int conditionIndex = 0;
    for (int i=0; i < block->length(); i++)
        if (block->get(i)->function == FUNCS.case_func)
            write_conditional_case(bc, block->get(i)->nestedContents, conditionIndex++);

    int switchFinishAddr = bc->opCount;
    
    // Update case-finish jumps
    resolve_unresolved_jump(bc, s_conditional_done, startAddr, switchFinishAddr);

    close_state_frame(bc, block, NULL);

    comment(bc, "conditional fin");
    end_minor_frame(bc);

    // Update liveness for term outputs
    for (int i=0;; i++) {
        Term* placeholder = get_output_placeholder(block, i);
        if (placeholder == NULL)
            break;
        Term* output = get_output_term(block->owningTerm, i);
        set_term_live(bc, output, firstOutputSlot + i);
    }
}

void func_call(Bytecode* bc, Term* term)
{
    comment(bc, "func call");

    int inputCount = term->numInputs() - 1;
    int top = reserve_new_frame_slots(bc, inputCount);

    append_op(bc, op_precall, top, inputCount);

    for (int i=0; i < inputCount; i++) {
        int inputSlot = top + 1 + i;
        load_input_term(bc, term, term->input(i + 1), inputSlot);
    }

    load_input_term(bc, term, term->input(0), top);
    append_op(bc, op_func_call_d, top, inputCount);
    set_term_live(bc, term, top);
}

void func_call_implicit(Bytecode* bc, Term* term)
{
    comment(bc, "func call (implicit)");

    int inputCount = term->numInputs();
    int top = reserve_new_frame_slots(bc, inputCount);

    append_op(bc, op_precall, top, inputCount);

    for (int i=0; i < inputCount; i++) {
        int inputSlot = top + 1 + i;
        load_input_term(bc, term, term->input(i), inputSlot);
    }

    load_input_term(bc, term, term->function, top);
    append_op(bc, op_func_call_d, top, inputCount);
    set_term_live(bc, term, top);
}

void func_apply(Bytecode* bc, Term* term)
{
    comment(bc, "func apply");

    int top = reserve_new_frame_slots(bc, 2);
    append_op(bc, op_precall, top, 2);

    load_input_term(bc, term, term->input(0), top + 1);
    load_input_term(bc, term, term->input(1), top + 2);
    append_op(bc, op_func_apply_d, top, 2);
    set_term_live(bc, term, top);
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
        int top = reserve_new_frame_slots(bc, inputCount);
        int firstTermIndex = find_first_closure_upvalue(block);
        append_op(bc, op_precall, top, inputCount);

        for (int i=0; i < inputCount; i++) {
            Term* upvalue = block->get(i + firstTermIndex);
            ca_assert(upvalue->function == FUNCS.upvalue);
            load_term(bc, upvalue->input(0), top + 1 + i, false);
        }

        call(bc, top, inputCount, FUNCS.make_list->nestedContents);

        bindingsSlot = top;
    }

    set_block(load_const(bc, blockSlot), block);
    //set_term_live(bc, 0, blockSlot);

    int resultSlot = reserve_slots(bc, 1);
    append_op(bc, op_make_func, resultSlot, blockSlot, bindingsSlot);
    set_term_live(bc, term, resultSlot);
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
    MinorFrame* mframe = bc->topMinorFrame;
    bool discard = exitType == s_discard;

    while (mframe->block != untilBlock) {

        if (mframe->hasStateFrame) {

            if (discard)
                append_op(bc, op_pop_discard_state_frame);
            else
                close_state_frame(bc, mframe->block, atTerm);

            if (is_loop(mframe->block)) {
                // Loop blocks have two state frames.
                if (discard)
                    append_op(bc, op_pop_discard_state_frame);
                else
                    write_pop_state_frame(bc);
            }
        }

        mframe = mframe->parent;
    }
}

void write_break(Bytecode* bc, Term* term)
{
    comment(bc, "break");
    Block* loop = find_enclosing_loop(term->owningBlock);
    pop_frames_for_early_exit(bc, s_break, term, loop);

    close_state_frame(bc, loop, term);
    loop_handle_locals_at_iteration_end(bc, loop, term, s_break);

    int addr = append_op(bc, op_jump);
    append_unresolved_jump(bc, addr, s_break);
}

void write_continue(Bytecode* bc, Term* term)
{
    comment(bc, "continue");
    Block* loop = find_enclosing_loop(term->owningBlock);
    pop_frames_for_early_exit(bc, s_continue, term, loop);

    close_state_frame(bc, loop, term);
    loop_advance_iterator(bc, loop);
    loop_handle_locals_at_iteration_end(bc, loop, term, s_continue);

    // investigate-
    // how to preserve the iteration result? real result might not be live,
    // we'd have to grab an intermediate value with the same name.
    //loop_preserve_iteration_result(bc, loop);

    int addr = append_op(bc, op_jump);
    append_unresolved_jump(bc, addr, s_continue);
}

void write_discard(Bytecode* bc, Term* term)
{
    comment(bc, "discard");
    Block* loop = find_enclosing_loop(term->owningBlock);
    pop_frames_for_early_exit(bc, s_discard, term, loop);

    if (should_write_state_header(bc, loop))
        append_op(bc, op_pop_discard_state_frame);
    loop_advance_iterator(bc, loop);
    loop_handle_locals_at_iteration_end(bc, loop, term, s_continue);

    int addr = append_op(bc, op_jump);
    append_unresolved_jump(bc, addr, s_continue);
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
    //set_term_live(bc, NULL, name_slot);

    int top = reserve_new_frame_slots(bc, 3);
    append_op(bc, op_precall, top, 3);
    append_op(bc, op_get_state_value, top+1, name_slot);
    load_input_term(bc, term, term->input(1), top + 2);
    load_input_term(bc, term, term->input(2), top + 3);

    call(bc, top, 3, FUNCS.declared_state->nestedContents);
    set_term_live(bc, term, top);
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
        set_term_live(bc, NULL, slot);
        load_term(bc, result, slot+1, false);
        append_op(bc, op_save_state_value, slot, slot+1);
    }
}

void close_state_frame(Bytecode* bc, Block* block, Term* atTerm)
{
    if (should_write_state_header(bc, block)) {
        save_declared_state(bc, block, atTerm);
        write_pop_state_frame(bc);
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
        || term_needs_no_evaluation(term)
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

    else if (term->function == FUNCS.switch_func || term->function == FUNCS.if_block)
        write_conditional_chain(bc, term->nestedContents);

    else if (term->function == FUNCS.func_call || term->function == FUNCS.func_call_method)
        func_call(bc, term);

    else if (!is_function(term->function))
        func_call_implicit(bc, term);

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
    reserve_slots(bc, count_input_placeholders(block) + 1);

    // Inputs are "live" at start of function.
    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            break;

        set_term_live(bc, placeholder, 1 + i);
    }

    for (int i=0;; i++) {
        Term* placeholder = get_input_placeholder(block, i);
        if (placeholder == NULL)
            break;

        int inputSlot = 1 + i;

        bool castNecessary = declared_type(placeholder) != TYPES.any;

        if (castNecessary)
            cast_fixed_type(bc, inputSlot, declared_type(placeholder));

        if (placeholder->boolProp(s_Multiple, false)) {
            // Collapse remaining args into a list.
            append_op(bc, op_varargs_to_list, i);
            break;
        }
    }

    int upvalueCount = count_closure_upvalues(block);
    if (upvalueCount > 0) {
        int first = reserve_slots(bc, upvalueCount);
        append_op(bc, op_splat_upvalues, first, upvalueCount);

        int firstUpvalue = find_first_closure_upvalue(block);
        for (int i=0; i < upvalueCount; i++) {
            Term* term = block->get(firstUpvalue + i);
            set_term_live(bc, term, first + i);
        }
    }

    // All input & closure slots are non-relocatable
    for (int i=0; i < bc->slotCount; i++)
        get_liveness(bc, i)->relocateable = false;
}

void major_block_contents(Bytecode* bc, Block* block)
{
    if (block_needs_no_evaluation(bc, block)) {
        comment(bc, "this block needs no evaluation");
        append_op(bc, op_ret_or_stop);
        return;
    }

    start_minor_frame(bc, block);
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
            if (minor_block_was_exited(bc))
                break;

            write_term(bc, block->get(i));
        }

        close_state_frame(bc, block, NULL);
        set_subroutine_output(bc, block, get_output_placeholder(block, 0)->input(0));
        append_op(bc, op_ret_or_stop);
    }

    end_minor_frame(bc);
}

void set_subroutine_output(Bytecode* bc, Block* block, Term* result)
{
    Term* placeholder = get_output_placeholder(block, 0);

    if (result == NULL) {
        append_op(bc, op_set_null, 0);
    } else {

        bool castNecessary = (declared_type(placeholder) != declared_type(result))
            && (declared_type(placeholder) != TYPES.any);

        load_term(bc, result, 0, false);
        //set_term_live(bc, result, 0);

        if (castNecessary)
            cast_fixed_type(bc, 0, declared_type(placeholder));
    }
}

int op_flags(int opcode)
{
    switch (opcode) {
    case op_uncompiled_call:
    case op_call:
    case op_func_call_s:
    case op_func_call_d:
    case op_dyn_method:
    case op_func_apply_d:
        return OP_READS_N_SLOTS | OP_WRITES_SLOT_A | OP_PUSHES_FRAME;
    case op_jump:
        return 0;
    case op_jif:
    case op_jnif:
        return OP_READS_SLOT_A;
    case op_jeq:
    case op_jneq:
    case op_jgt:
    case op_jgte:
    case op_jlt:
    case op_jlte:
        return OP_READS_SLOT_A | OP_READS_SLOT_B;
    case op_ret:
    case op_ret_or_stop:
    case op_grow_frame:
        return 0;
    case op_load_const:
    case op_load_i:
    case op_varargs_to_list:
        return OP_WRITES_SLOT_A;
    case op_splat_upvalues:
    case op_native:
        return 0;
    case op_copy:
    case op_move:
        return OP_READS_SLOT_B | OP_WRITES_SLOT_A;
    case op_set_null:
        return OP_WRITES_SLOT_A;
    case op_cast_fixed_type:
        return OP_READS_SLOT_B | OP_WRITES_SLOT_A;
    case op_add_i:
    case op_sub_i:
    case op_mult_i:
    case op_div_i:
    case op_make_func:
        return OP_WRITES_SLOT_A | OP_READS_SLOT_B | OP_READS_SLOT_C;
    case op_push_state_frame:
        return 0;
    case op_push_state_frame_dkey:
        return OP_READS_SLOT_A;
    case op_pop_state_frame:
    case op_pop_discard_state_frame:
        return 0;
    case op_get_state_value:
        return OP_READS_SLOT_B | OP_WRITES_SLOT_A;
    case op_save_state_value:
        return OP_READS_SLOT_A | OP_READS_SLOT_B;
    case op_comment:
    default:
        return 0;
    }
}

void perform_move_optimization(Bytecode* bc)
{
    // Convert op_copy to op_move when it's safe to do so.
    // Bytecode is still in SSA form.
    
    int copyCount = 0;
    int convertedCount = 0;
    
    for (int pc=0; pc < bc->opCount; pc++) {
        Op* op = &bc->ops[pc];

        if (op->opcode == op_copy) {
            copyCount++;
            Liveness* al = get_liveness(bc, op->b);
            if (al->lastReadPc <= pc) {
                op->opcode = op_move;
                convertedCount++;
            }
        }
    }

#if TRACE_MOVE_OPTIMIZATION
    if (copyCount > 0) {
        printf("move optimization: converted %.1f%% of %d op_copys to op_move\n",
            100.0 * convertedCount / (copyCount), copyCount);
    }
#endif
}

struct SlotCompaction {

    Bytecode* bc;
    int* new_slot_to_old;
    int* old_slot_to_new;
    int newSlotCount;

    ~SlotCompaction()
    {
        free(new_slot_to_old);
        free(old_slot_to_new);
    }

    void init(Bytecode* _bc)
    {
        bc = _bc;
        newSlotCount = 0;

        new_slot_to_old = (int*) malloc(sizeof(int) * bc->slotCount);
        old_slot_to_new = (int*) malloc(sizeof(int) * bc->slotCount);

        for (int i=0; i < bc->slotCount; i++) {
            new_slot_to_old[i] = -1;
            old_slot_to_new[i] = -1;
        }
    }

    bool is_new_slot_unused(int newSlot, int pc)
    {
        ca_assert(newSlot >= 0);

        if (new_slot_to_old[newSlot] == -1)
            // Haven't used this slot yet.
            return true;

        Liveness* liveness = get_liveness(bc, new_slot_to_old[newSlot]);

        if ((pc > liveness->lastReadPc) && (pc > liveness->writePc))
            // This slot was used in a remapping, but that liveness is dead.
            // Note that we don't try to use slots *before* their liveness, only after.
            return true;

        return false;
    }

    void save_remap(int old, int newSlot)
    {
        new_slot_to_old[newSlot] = old;
        old_slot_to_new[old] = newSlot;
        if (newSlot >= newSlotCount)
            newSlotCount = newSlot+1;
    }

    int get_first_available_new_slot(int pc, int old)
    {
        for (int newSlot=0; newSlot < bc->slotCount; newSlot++) {
            if (!get_liveness(bc, newSlot)->relocateable)
                continue;

            if (is_new_slot_unused(newSlot, pc))
                return newSlot;
        }

        internal_error("unexpected fallthrough in SlotCompaction.get_first_available_new_slot");
        return 0;
    }

    void do_init_pass()
    {
        // Don't move slots that are not 'relocateable'
        for (int slot=0; slot < bc->slotCount; slot++)
            if (!get_liveness(bc, slot)->relocateable)
                save_remap(slot, slot);
    }

    void do_calculate_pass() 
    {
        int validRange = 0;

        while ((validRange < bc->slotCount) && old_slot_to_new[validRange] != -1)
            validRange++;

        for (int pc=0; pc < bc->opCount; pc++) {
            Op op = bc->ops[pc];
            int opflags = op_flags(op.opcode);

            if (op.opcode == op_precall) {
                // Reduce 'validRange' if possible
                while (validRange > 0 && is_new_slot_unused(validRange-1, pc))
                    validRange--;

                validRange += 2;

                for (int i=0; i < op.b + 1; i++)
                    save_remap(op.a + i, validRange + i);

                validRange += op.b + 1;
                continue;
            }

            if (!(opflags & OP_WRITES_SLOT_A))
                continue;

            if (old_slot_to_new[op.a] != -1)
                // Already remapped (maybe was not relocatable, or maybe relocated by op_precall)
                continue;

            // Call ops should have already been remapped (when we find the op_precall)
            ca_assert(!(opflags & OP_PUSHES_FRAME));

            // Not a call op, the write slot can simply be the 1st available.
            int newSlot = get_first_available_new_slot(pc, op.a);
            if (newSlot >= validRange)
                validRange = newSlot + 1;

            save_remap(op.a, newSlot);
        }
    }

    void do_commit_pass()
    {
        for (int pc=0; pc < bc->opCount; pc++) {
            Op* op = &bc->ops[pc];
            int opflags = op_flags(op->opcode);

            if ((opflags & OP_READS_SLOT_A)
                    || (opflags & OP_WRITES_SLOT_A)
                    || (opflags & OP_READS_N_SLOTS)
                    || (op->opcode == op_precall)) {
                ca_assert(old_slot_to_new[op->a] != -1);
                op->a = old_slot_to_new[op->a];
            }

            if (opflags & OP_READS_SLOT_B) {
                ca_assert(old_slot_to_new[op->b] != -1);
                op->b = old_slot_to_new[op->b];
            }

            if (opflags & OP_READS_SLOT_C) {
                ca_assert(old_slot_to_new[op->c] != -1);
                op->c = old_slot_to_new[op->c];
            }
        }

        bc->slotCount = newSlotCount;
    }

    void dump()
    {
        printf("SlotCompaction dump:\n");
        for (int i=0; i < bc->slotCount; i++) {
            int newSlot = old_slot_to_new[i];
            if (newSlot == -1) {
                printf("  r%d -> unused\n", i);
            } else {
                printf("  r%d -> r%d\n", i, newSlot);
            }
        }
    }
};

void perform_slot_compaction(Bytecode* bc)
{
    // Convert SSA bytecode into bytecode that efficiently reuses slots.
    
    SlotCompaction map;
    map.init(bc);
    map.do_init_pass();
    map.do_calculate_pass();

    #if TRACE_SLOT_COMPACTION
        printf("-- Slot compaction calculated --\n");
        bc->dump();
        map.dump();

        printf("Committed:\n");
        map.do_commit_pass();
        bc->dump();
    #else
        map.do_commit_pass();
    #endif
}

void perform_ops_prune(Bytecode* bc)
{
    for (int pc=0; pc < bc->opCount; pc++) {
        Op* op = &bc->ops[pc];

        if ((op->opcode == op_move || op->opcode == op_copy) && op->a == op->b)
            op->opcode = op_nope;

        if (op->opcode == op_precall)
            op->opcode = op_nope;
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

    // Optimization
    perform_move_optimization(bc);
    perform_slot_compaction(bc);
    perform_ops_prune(bc);

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
    case op_load_const:
        op->b += constDelta;
        break;
    case op_comment:
        op->a += constDelta;
        break;
    case op_cast_fixed_type:
        op->c += constDelta;
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
        printf("uncompiled_call top:r%d count:%d const:%d\n", op.a, op.b , op.c);
        break;
    case op_call:
        printf("call top:r%d count:%d addr:%d\n", op.a, op.b , op.c);
        break;
    case op_func_call_d:
        printf("func_call_d top:r%d count:%d func:%d\n", op.a, op.b, op.c);
        break;
    case op_func_apply_d:
        printf("func_apply_d top:r%d count:%d func:r%d\n", op.a, op.b, op.c);
        break;
    case op_dyn_method:
        printf("dyn_method top:r%d count:%d nameLocationConst:%d\n", op.a, op.b, op.c);
        break;
    case op_precall:
        printf("precall top:r%d count:%d\n", op.a, op.b);
        break;
    case op_jump: printf("jump addr:%d\n", op.c); break;
    case op_jif: printf("jif r%d addr:%d\n", op.a, op.c); break;
    case op_jnif: printf("jnif r%d addr:%d\n", op.a, op.c); break;
    case op_jeq: printf("jeq r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_jneq: printf("jneq r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_jgt: printf("jgt r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_jgte: printf("jgte r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_jlt: printf("jlt r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_jlte: printf("jlte r%d r%d addr:%d\n", op.a, op.b, op.c); break;
    case op_grow_frame: printf("grow_frame %d\n", op.a); break;
    case op_load_const: printf("load_const slot:r%d const:%d\n", op.a, op.b); break;
    case op_load_i: printf("load_i r%d value:%d\n", op.a, op.b); break;
    case op_native: printf("native %d\n", op.a); break;
    case op_ret: printf("ret\n"); break;
    case op_ret_or_stop: printf("ret_or_stop\n"); break;
    case op_varargs_to_list: printf("varargs_to_list first:r%d\n", op.a); break;
    case op_splat_upvalues: printf("splat_upvalues first:r%d count:%d\n", op.a, op.b); break;
    case op_copy: printf("copy r%d to r%d\n", op.b, op.a); break;
    case op_move: printf("move r%d to r%d\n", op.b, op.a); break;
    case op_set_null: printf("set_null r%d\n", op.a); break;
    case op_cast_fixed_type: printf("cast_fixed_type dest:r%d r%d const:%d\n", op.a, op.b, op.c); break;
    case op_make_func: printf("make_func dest:r%d r%d r%d\n", op.a, op.b, op.c); break;
    case op_add_i: printf("add_i dest:r%d r%d r%d\n", op.a, op.b, op.c); break;
    case op_sub_i: printf("sub_i dest:r%d r%d r%d\n", op.a, op.b, op.c); break;
    case op_mult_i: printf("mult_i dest:r%d r%d r%d\n", op.a, op.b, op.c); break;
    case op_div_i: printf("div_i dest:r%d r%d r%d\n", op.a, op.b, op.c); break;
    case op_push_state_frame: printf("push_state_frame (static key)\n"); break;
    case op_push_state_frame_dkey: printf("push_state_frame_dkey r%d\n", op.a); break;
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
    for (int pc=0; pc < bc->opCount; pc++) {
        printf("  %d: ", pc);
        dump_op(bc, bc->ops[pc]);
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

    Value* blockRecord = assembled->blockToAddr.insert_val(&blockVal);
    blockRecord->set_list(2);
    blockRecord->index(0)->set_int(baseOp);
    blockRecord->index(1)->set_int(assembled->opCount);

    #if DUMP_COMPILED_BYTECODE
        bc->dump();
    #endif

    free_bytecode(bc);

    return baseOp;
}

int find_compiled_major_block(Bytecode* bc, Block* block)
{
    Value blockVal;
    set_block(&blockVal, block);

    Value* found = bc->blockToAddr.val_key(&blockVal);

    if (found != NULL)
        return found->index(0)->as_i();

    return -1;
}

int find_or_compile_major_block(Bytecode* bc, Block* block)
{
    Value blockVal;
    set_block(&blockVal, block);

    Value* found = bc->blockToAddr.val_key(&blockVal);

    if (found != NULL)
        return found->index(0)->as_i();
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
        || mopcode == mop_minor_block_end
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
        //case mop_term_eval_end:
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
