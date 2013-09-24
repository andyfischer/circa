// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "stack.h"
#include "loops.h"
#include "object.h"
#include "tagged_value.h"
#include "term_list.h"

namespace circa {

struct Stack
{
    // GCable object header.
    CircaObject header;

    // Globally unique ID.
    int id;

    // Frame list
    Frame* frames;
    int framesCount;
    int framesCapacity;

    // Current step, either StackReady, StackRunning or StackFinished.
    Symbol step;

    // Flag that indicates the most recent run was interrupted by an error
    bool errorOccurred;

    // Linked list of all stacks across this world.
    Stack* prevStack;
    Stack* nextStack;

    // Owning world
    caWorld* world;

    // Value slot, may be used by the stack's owner.
    Value context;

    Stack();
    ~Stack();

    void dump();

private:
    // Disabled C++ functions.
    Stack(Stack const&) {}
    Stack& operator=(Stack const&) { return *this; }
};

// Allocate a new Stack object.
Stack* create_stack(World* world);
void free_stack(Stack* stack);

// -- Stack --
Frame* stack_top(Stack* stack);
Frame* stack_top_parent(Stack* stack);
Block* stack_top_block(Stack* stack);

// (Re)initialize the stack to have just one frame, using 'main' as its block. Existing data
// is erased.
void stack_init(Stack* stack, Block* main);

// Push a frame onto the stack.
Frame* stack_push(Stack* stack, Block* block);

// Pop the topmost frame from the stack.
void stack_pop(Stack* stack);

// Reset a Stack to its default value.
void stack_reset(Stack* stack);

// Pop all but the topmost frame, set the PC to the first term, and delete all temporary
// values. If there is a state register, feed the output back into its input.
void stack_restart(Stack* stack);

void stack_restart_at(Stack* stack, int termIndex);

// Clear the error flag, but leave the stack as-is. See also stack_clear_error.
void stack_ignore_error(Stack* stack);

// Clear the error flag and drop any intermediate stack frames. The stack will be
// cleared up until the 'stop' frame (as if it successfully finished an evaluation).
void stack_clear_error(Stack* stack);

caValue* stack_get_state(Stack* stack);

caValue* stack_find_active_value(Frame* frame, Term* term);

// Returns whether evaluation has been stopped due to an error.
bool stack_errored(Stack* stack);

void stack_to_string(Stack* stack, caValue* out);
void stack_trace_to_string(Stack* stack, caValue* out);

void stack_extract_state(Stack* stack, caValue* output);

// -- Frame --

Frame* frame_parent(Frame* frame);
Term* frame_caller(Frame* frame);
Term* frame_term(Frame* frame, int index);
caValue* frame_register(Frame* frame, int index);
caValue* frame_register(Frame* frame, Term* term);
caValue* frame_register_from_end(Frame* frame, int index);
int frame_register_count(Frame* frame);
caValue* frame_registers(Frame* frame);
caValue* frame_bytecode(Frame* frame);
Block* frame_block(Frame* frame);
int frame_get_index(Frame* frame);

void frame_extract_state(Frame* frame, caValue* output);

Frame* frame_by_index(Stack* stack, int index);
int stack_frame_count(Stack* stack);

// Retrieve the frame with the given depth.
Frame* frame_by_depth(Stack* stack, int depth);

// Run the interpreter.
void run_interpreter(Stack* stack);

void stack_run(Stack* stack);

void run_bytecode(Stack* stack, caValue* bytecode);

// Deprecated
void evaluate_block(Stack* stack, Block* block);

// Copy all of the outputs from the topmost frame. This is an alternative to finish_frame
// - you call it when the block is finished evaluating. But instead of passing outputs
// to the parent frame (like finish_frame does), this copies them to your list.
void fetch_stack_outputs(Stack* stack, caValue* outputs);

// Functions used by eval functions.
caValue* get_input(Stack* stack, int index);
void consume_input(Stack* stack, Term* term, caValue* dest);
void consume_input(Stack* stack, int index, caValue* dest);
int num_inputs(Stack* stack);
void consume_inputs_to_list(Stack* stack, List* list);
caValue* get_output(Stack* stack, int index);
caValue* get_caller_output(Stack* stack, int index);

Term* current_term(Stack* stack);
Block* current_block(Stack* stack);

// Get a register on the topmost frame.
caValue* get_top_register(Stack* stack, Term* term);

// Miscellaneous stack access & manipulation.
caValue* context_inject(Stack* stack, caValue* name);

// Create an output value for the current term, using the declared type's
// initialize function.
void create_output(Stack* stack);

// Signal that a runtime error has occurred.
void raise_error(Stack* stack);
void raise_error_msg(Stack* stack, const char* msg);
void raise_error_too_many_inputs(Stack* stack);
void raise_error_not_enough_inputs(Stack* stack);
void raise_error_input_type_mismatch(Stack* stack);

// Kernel setup.
void interpreter_install_functions(Block* block);

bool is_stack(caValue* value);
Stack* as_stack(caValue* value);
void set_stack(caValue* value, Stack* stack);

} // namespace circa
