// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// bytecode2
//
// goals:
//
//   Support function inlining
//   Support assumptions
//     Lazily generate type-specialized versions of basic blocks on-the-fly


#pragma once

#include "value_array.h"

namespace circa {

const char op_nope = 0x1;

const char op_uncompiled_call = 0x3;  // a: top, b: inputCount, c: const index of block
const char op_call = 0x8;             // a: top, b: inputCount, c: addr
const char op_func_call_s = 0x43;     // a: top, b: inputCount, c: addr
const char op_func_call_d = 0x2;      // a: top, b: inputCount
const char op_func_apply_d = 0x39;    // a: top & slot of input func
const char op_dyn_method = 0x38;      // a: top, b: inputCount, c: const index of [name,location]

const char op_jump = 0x4;             // c: addr
const char op_jif = 0x30;             // a: slot, c: addr
const char op_jnif = 0x31;            // a: slot, c: addr
const char op_jeq = 0x24;             // a: leftSlot, b: rightSlot, c: addr
const char op_jneq = 0x25;            // a: leftSlot, b: rightSlot, c: addr
const char op_jgt = 0x26;             // a: leftSlot, b: rightSlot, c: addr
const char op_jgte = 0x27;            // a: leftSlot, b: rightSlot, c: addr
const char op_jlt = 0x28;             // a: leftSlot, b: rightSlot, c: addr
const char op_jlte = 0x29;            // a: leftSlot, b: rightSlot, c: addr

const char op_ret = 0x5;
const char op_ret_or_stop = 0x18;

const char op_grow_frame = 0x16;      // a: size
const char op_load_const = 0x15;      // a: constIndex, b: destSlot
const char op_load_i = 0x45;          // a: value, b: destSlot
const char op_varargs_to_list = 0x14; // a: slot & inputIndex
const char op_splat_upvalues = 0x42;  // a: firstSlot, b: count

const char op_native = 0x17;          // a: nativeFuncIndex

const char op_copy = 0x12;            // a: sourceSlot, b: destSlot
const char op_move = 0x7;             // a: sourceSlot, b: destSlot
const char op_set_null = 0x6;         // a: slot
const char op_cast_fixed_type = 0x40; // a: slot, b: const index of type
const char op_make_func = 0x13;       // a: blockSlot (and output), b: bindingSlot
const char op_make_list = 0x41;       // a: first, b: count

const char op_add_i = 0x20;           // a: left, b: right, c: dest
const char op_sub_i = 0x21;
const char op_mult_i = 0x22;
const char op_div_i = 0x23;

const char op_push_state_frame = 0x35; // a: frameSlot
const char op_push_state_frame_dkey = 0x47; // a: frameSlot, b: keySlot
const char op_pop_state_frame = 0x36;
const char op_pop_discard_state_frame = 0x46;
const char op_get_state_value = 0x37; // a: key, b: dest
const char op_save_state_value = 0x44; // a: key, b: value

const char op_comment = 0x11;         // a: const index of comment

const char mop_term_eval_start = 0x1;

// term_eval_end(term, mopaddr) - the mopaddr points to the matching term_eval_start
const char mop_term_eval_end = 0x2;
const char mop_term_live = 0x3;
const char mop_major_block_start = 0x4;
const char mop_major_block_end = 0x5;
const char mop_minor_block_start = 0x6;
const char mop_minor_block_end = 0x7;

struct Op {
    u16 a : 16;
    u16 b : 16;
    u16 c : 16;
    u16 opcode : 16;
};

const int OP_READS_SLOT_A = 0x1;
const int OP_READS_SLOT_B = 0x2;
const int OP_READS_SLOT_C = 0x4;
const int OP_WRITES_SLOT_A = 0x8;
const int OP_WRITES_SLOT_B = 0x10;
const int OP_READS_N_SLOTS = 0x40;

struct BytecodeMetadata {
    u32 mopcode;
    u32 addr;
    u32 slot;
    u32 related_maddr;
    union {
        Term* term;
        Block* block;
    };
};

struct Liveness {
    Term* term;       // may be NULL
    int firstWritePc; // may be -1
    int lastReadPc;   // may be -1
};

struct MinorBlock {
    MinorBlock* parent;
    bool hasBeenExited;
    int firstTemporarySlot;
};

struct Bytecode {

    VM* vm;

    ValueArray consts;

    int opCount;
    Op* ops;

    int metadataSize;
    BytecodeMetadata* metadata;

    // Used for in-progress major block compilation:
    Value unresolved; // list of {addr, type}
    Value minorBlockInfo; // map of block -> {slot}
    int livenessCount;
    Liveness* liveness;
    MinorBlock* currentMinorBlock;

    int nextFreeSlot;
    int slotCount;

    // Used for an assembled program:
    Value blockToAddr; // map of major block -> int address

    void dump();
    void dump_metadata();
};

void free_bytecode(Bytecode* bc);
void exec(Block* block);
Value* get_const(Bytecode* bc, int index);
Term* find_active_term(Bytecode* bc, int addr);
Block* find_active_major_block(Bytecode* bc, int addr);
int find_or_compile_major_block(Bytecode* bc, Block* block);
void vm_prepare_bytecode(VM* vm, VM* callingVM);
void vm_run(VM* vm, VM* callingVM);

// Bytecode metadata
int find_metadata_addr_for_addr(Bytecode* bc, int addr);
int mop_search_skip_minor_blocks(Bytecode* bc, int maddr);
int mop_find_active_mopcode(Bytecode* bc, int mopcode, int maddr);
bool mopcode_uses_related_maddr(int mopcode);
//Block* find_active_minor_block(Bytecode* bc, int addr);
//int find_active_state_key_slot(Bytecode* bc, int addr);
//int find_active_state_header_slot(Bytecode* bc, int addr);
void dump_op(Bytecode* bc, Op op);

} // namespace circa
