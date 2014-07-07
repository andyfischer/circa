// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "stack.h"
#include "loops.h"
#include "tagged_value.h"
#include "term_list.h"

namespace circa {

// -- Stack --

// (Re)initialize the stack to have just one frame, using 'main' as its block. Existing data
// is erased.
void stack_init(Stack* stack, Block* main);

void stack_init_with_closure(Stack* stack, caValue* closure);

// Reset a Stack to its default value.
void stack_reset(Stack* stack);

// Pop all but the topmost frame, set the PC to the first term, and delete all temporary
// values. If there is a state register, feed the output back into its input.
void stack_restart(Stack* stack);
void stack_restart_discarding_state(Stack* stack);

caValue* stack_get_state(Stack* stack);

caValue* stack_find_nonlocal(Frame* frame, Term* term);

void stack_to_string(Stack* stack, caValue* out, bool withBytecode);
void stack_trace_to_string(Stack* stack, caValue* out);

void vm_run(Stack* stack);
Frame* vm_push_frame2(Stack* stack, int parentIndex, int blockIndex);
int vm_compile_block(Stack* stack, Block* block);

// Deprecated
void evaluate_block(Stack* stack, Block* block);

// Copy all of the outputs from the topmost frame. This is an alternative to finish_frame
// - you call it when the block is finished evaluating. But instead of passing outputs
// to the parent frame (like finish_frame does), this copies them to your list.
void fetch_stack_outputs(Stack* stack, caValue* outputs);

// Functions used by eval functions.
int num_inputs(Stack* stack);
void consume_inputs_to_list(Stack* stack, Value* list);

Term* current_term(Stack* stack);
Block* current_block(Stack* stack);

// Get a register on the topmost frame.
caValue* get_top_register(Stack* stack, Term* term);

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
void interpreter_install_functions(NativePatch* patch);

bool is_stack(caValue* value);
Stack* as_stack(caValue* value);
void set_stack(caValue* value, Stack* stack);

} // namespace circa
