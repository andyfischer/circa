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

Frame* as_frame_ref(caValue* value);
bool is_frame_ref(caValue* value);
void set_frame_ref(caValue* value, Frame* frame);

void set_retained_frame(caValue* frame);
bool is_retained_frame(caValue* frame);
Block* retained_frame_get_block(caValue* frame);
caValue* retained_frame_get_state(caValue* frame);

void copy_stack_frame_to_retained(Frame* source, caValue* retainedFrame);

void frame_copy(Frame* left, Frame* right);
// void copy_stack_frame_to_boxed(Frame* frame, caValue* value);

// void frame_setup_type(Type* type);

} // namespace circa
