// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Frame
{
    // Pointer to owning Stack.
    Stack* stack;

    // PC (in the parent frame) that this frame was expanded from. Invalid for bottom frame.
    int parentPc;

    // The role or state of this frame.
    Symbol role;

    // Register values.
    Value registers;

    // Stored frame state.
    Value state;

    // Source block
    Block* block;

    // Which version of the block we are using.
    // TODO: Remove. (instead, code changes should create different blocks)
    int blockVersion;

    Value customBytecode;

    Value dynamicScope;

    // Current program counter (term index)
    int pcIndex;

    // Program counter (bytecode position)
    int pc;

    // Whether this frame was pushed from a normal call, or Func.apply/Func.call.
    Symbol callType;

    // When a block is exited early, this stores the exit type.
    Symbol exitType;

    // Whether to save this frame on completion.
    bool retain;
};

Frame* as_frame(caValue* value);
bool is_frame(caValue* value);
void set_frame(caValue* value);

void frame_copy(Frame* left, Frame* right);
void copy_stack_frame_to_boxed(Frame* frame, caValue* value);

void frame_setup_type(Type* type);

} // namespace circa
