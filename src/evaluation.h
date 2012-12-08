// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "dict.h"
#include "list.h"
#include "loops.h"
#include "gc.h"
#include "tagged_value.h"
#include "term_list.h"

namespace circa {

typedef int FrameId;

struct Frame
{
    // Frame ID, this is unique across the Stack.
    FrameId id;

    // Pointer to owning Stack.
    Stack* stack;

    // ID of this frame's parent. The bottommost frame has parent 0. In the free list, this
    // field masquarades as the "next free frame id".
    FrameId parent;

    // PC (in the parent frame) that this frame was expanded from. Invalid for bottom frame.
    int parentPc;

    // The role or state of this frame.
    caName role;

    // Register values.
    List registers;

    // Bytecode
    Value bytecode;

    // Source block
    Block* block;

    // Which version of the block we are using.
    int blockVersion;

    // Current program counter
    int pc;
    int nextPc;

    // When a block is exited early, this stores the exit type.
    caName exitType;
    
    // If true, the interpreter should stop after completing this frame. The frame
    // will be left on the stack.
    bool stop;
};

struct Stack
{
    // GCable object header.
    CircaObject header;

    // Globally unique ID.
    int id;

    // Frame list
    int framesCapacity;
    Frame* frames;

    // Topmost frame in the current execution state.
    FrameId top;

    // First free frame entry. In the frame list, there is a shadow list of free frames.
    FrameId firstFreeFrame;
    FrameId lastFreeFrame;

    // Whether the interpreter is currently running. Set to false when an error occurs
    // or when the block is completed.
    bool running;

    // Flag that indicates the most recent run was interrupted by an error
    bool errorOccurred;

    // Top-level state (deprecated)
    Value state;

    // Owning world
    caWorld* world;

    // Value slot, may be used by the stack's owner.
    Value context;

    Stack();
    ~Stack();

    void dump();

private:
    // Disabled C++ funcs.
    Stack(Stack const&) {}
    Stack& operator=(Stack const&) { return *this; }
};

// Allocate a new Stack object.
Stack* alloc_stack(World* world);

// *** High-level Stack manipulation ***

// Access the stack.
Frame* top_frame(Stack* stack);
Frame* top_frame_parent(Stack* stack);
Block* top_block(Stack* stack);

// Retrieve the frame with the given depth, this function is O(n).
Frame* frame_by_depth(Stack* stack, int depth);

// Run the interpreter.
void run_interpreter(Stack* stack);
void run_interpreter_step(Stack* stack);
void run_interpreter_steps(Stack* stack, int steps);

// Evaluate a single term. Deprecated.
void evaluate_single_term(Stack* stack, Term* term);
void evaluate_block(Stack* stack, Block* block);

// Evaluate only the terms between 'start' and 'end'.
void evaluate_range(Stack* stack, Block* block, int start, int end);

// Evaluate 'term' and every term that it depends on. 
void evaluate_minimum(Stack* stack, Term* term, caValue* result);
void evaluate_minimum2(Term* term, caValue* output);

// Returns whether evaluation has been interrupted, such as with a 'return' or
// 'break' statement, or a runtime error.
bool error_occurred(Stack* stack);

// Clear the error flag, but leave the stack as-is. See also stack_clear_error.
void stack_ignore_error(Stack* stack);

// Clear the error flag and drop any intermediate stack frames. The stack will be
// cleared up until the 'stop' frame (as if it successfully finished an evaluation).
void stack_clear_error(Stack* stack);

// Reset a Stack to its default value.
void reset_stack(Stack* stack);

// Push a frame onto the stack.
Frame* push_frame(Stack* stack, Block* block);
Frame* push_frame_with_inputs(Stack* stack, Block* block, caValue* inputs);

// Pop the topmost frame and throw it away. This call doesn't preserve the frame's
// outputs or update PC. You might want to call finish_frame() instead of this.
void pop_frame(Stack* stack);

// Flag this frame to stop the interpreter when finished.
void frame_set_stop_when_finished(Frame* frame);

// Copy all of the outputs from the topmost frame. This is an alternative to finish_frame
// - you call it when the block is finished evaluating. But instead of passing outputs
// to the parent frame (like finish_frame does), this copies them to your list.
void fetch_stack_outputs(Stack* stack, caValue* outputs);

// Pop the topmost frame and copy all outputs to the next frame on the stack. This is the
// standard way to finish a frame, such as when 'return' is called.
void finish_frame(Stack* stack);

// Move the PC to the end of the frame.
void frame_pc_move_to_end(Frame* frame);

// Stack expansions. These are frames which aren't on the current trace.
Frame* stack_expand_call(Stack* stack, Frame* frame, Term* term);

// Functions used by eval functions.
caValue* get_input(Stack* stack, int index);
caValue* find_stack_value_for_term(Stack* stack, Term* term, int stackDelta);
void consume_input(Stack* stack, Term* term, caValue* dest);
void consume_input(Stack* stack, int index, caValue* dest);
int num_inputs(Stack* stack);
void consume_inputs_to_list(Stack* stack, List* list);
caValue* get_output(Stack* stack, int index);
caValue* get_caller_output(Stack* stack, int index);

Term* current_term(Stack* stack);
Block* current_block(Stack* stack);

// Registers
caValue* get_frame_register(Frame* frame, int index);
caValue* get_frame_register(Frame* frame, Term* term);
caValue* get_frame_register_from_end(Frame* frame, int index);
int frame_register_count(Frame* frame);
caValue* frame_registers(Frame* frame);

// Get a register on the topmost frame.
caValue* get_top_register(Stack* stack, Term* term);

caValue* frame_bytecode(Frame* frame);

EvaluateFunc get_override_for_block(Block* block);

// Create an output value for the current term, using the declared type's
// initialize function.
void create_output(Stack* stack);

// Signal that a runtime error has occurred.
void raise_error(Stack* stack);
void raise_error_msg(Stack* stack, const char* msg);

void print_stack(Stack* stack, std::ostream& out);
void print_error_stack(Stack* stack, std::ostream& out);

// Update bytecode
void write_term_bytecode(Term* term, caValue* output);
void write_block_bytecode(Block* block, caValue* output);

// Setup the builtin Stack type.
void eval_context_setup_type(Type* type);

void interpreter_install_functions(Block* block);

} // namespace circa
