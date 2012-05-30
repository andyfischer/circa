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

struct Frame
{
    List registers;
    Branch* branch;
    int pc;
    int nextPc;
    int endPc;

    // True if this frame is being evaluated with a C override.
    bool override;

    // True if this frame is being called by dynamic_call
    bool dynamicCall;

    // Used in for-loop
    bool loop;

    // When a branch is exited early, this stores the exit type.
    caName exitType;
    
    // If true, the interpreter should stop after completing this frame. The frame
    // will be left on the stack.
    bool stop;
};

struct Stack
{
    CircaObject header;

    // Top-level state (deprecated)
    Value state;

    // Current execution stack
    int numFrames;
    Frame* stack;

    // Whether the interpreter is currently running. Set to false when an error occurs
    // or when the branch is completed.
    bool running;

    // Flag that indicates the most recent run was interrupted by an error
    bool errorOccurred;

    // Owning world
    caWorld* world;

    Stack();
    ~Stack();

private:
    // Disabled calls
    Stack(Stack const&) {}
    Stack& operator=(Stack const&) { return *this; }
};

Stack* alloc_stack(World* world);

void eval_context_print_multiline(std::ostream& out, Stack* stack);
void eval_context_setup_type(Type* type);

// Stack frames
Frame* get_frame(Stack* stack, int depth);
Frame* get_frame_from_bottom(Stack* stack, int index);

// Push a new frame with the given registers.
Frame* push_frame(Stack* stack, Branch* branch, List* registers);

Frame* push_frame(Stack* stack, Branch* branch);
void push_frame_with_inputs(Stack* stack, Branch* branch, caValue* inputs);

// Pop the topmost frame and throw it away. This call doesn't preserve the frame's
// outputs or update PC. You might want to call finish_frame() instead of this.
void pop_frame(Stack* stack);

void frame_set_stop_when_finished(Frame* frame);

// Copy all of the outputs from the topmost frame. This is an alternative to finish_frame
// - you call it when the branch is finished evaluating. But instead of passing outputs
// to the parent frame (like finish_frame does), this copies them to your list.
void fetch_stack_outputs(Stack* stack, caValue* outputs);

// Pop the topmost frame and copy all outputs to the next frame on the stack. This is the
// standard way to finish a frame, such as when 'return' is called.
void finish_frame(Stack* stack);

Frame* top_frame(Stack* stack);
Branch* top_branch(Stack* stack);
void reset_stack(Stack* stack);

// Evaluate a single term. This is not usually called directly, it's called
// by the interpreter.
void evaluate_single_term(Stack* stack, Term* term);

void evaluate_branch(Stack* stack, Branch* branch);

void evaluate_save_locals(Stack* stack, Branch* branch);

// Shorthand to call evaluate_branch with a new Stack:
void evaluate_branch(Branch* branch);

// Evaluate only the terms between 'start' and 'end'.
void evaluate_range(Stack* stack, Branch* branch, int start, int end);

// Evaluate 'term' and every term that it depends on.
void evaluate_minimum(Stack* stack, Term* term, caValue* result);

// Parse input and immediately evaluate it, returning the result value.
caValue* evaluate(Stack* stack, Branch* branch, std::string const& input);
caValue* evaluate(Branch* branch, Term* function, List* inputs);
caValue* evaluate(Term* function, List* inputs);

void insert_explicit_inputs(Stack* stack, caValue* inputs);
void extract_explicit_outputs(Stack* stack, caValue* inputs);

caValue* get_input(Stack* stack, int index);
caValue* find_stack_value_for_term(Stack* stack, Term* term, int stackDelta);
void consume_input(Stack* stack, Term* term, caValue* dest);
void consume_input(Stack* stack, int index, caValue* dest);
int num_inputs(Stack* stack);
void consume_inputs_to_list(Stack* stack, List* list);
caValue* get_output(Stack* stack, int index);
caValue* get_caller_output(Stack* stack, int index);

Term* current_term(Stack* stack);
Branch* current_branch(Stack* stack);
caValue* get_frame_register(Frame* frame, int index);
caValue* get_frame_register_from_end(Frame* frame, int index);
caValue* get_register(Stack* stack, Term* term);
EvaluateFunc get_override_for_branch(Branch* branch);

// Create an output value for the current term, using the declared type's
// initialize function.
void create_output(Stack* stack);

// Signal that a runtime error has occurred.
void raise_error(Stack* stack);

void raise_error_msg(Stack* stack, const char* msg);

// Returns whether evaluation has been interrupted, such as with a 'return' or
// 'break' statement, or a runtime error.
bool error_occurred(Stack* stack);

void clear_error(Stack* cxt);

void print_error_stack(Stack* stack, std::ostream& out);

void run_interpreter(Stack* stack);
void run_interpreter_step(Stack* stack);
void run_interpreter_steps(Stack* stack, int steps);

// Dynamic calls
void dynamic_call_func(caStack* stack);
void finish_dynamic_call(caStack* stack);

} // namespace circa
