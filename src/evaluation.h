// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "dict.h"
#include "for_loop.h"
#include "gc.h"
#include "tagged_value.h"
#include "term_list.h"
#include "types/list.h"

namespace circa {

struct Frame
{
    List registers;
    Branch* branch;
    int pc;
};

struct EvalContext
{
    CircaObject header;

    // Error information:
    bool errorOccurred;
    Term* errorTerm;

    // Persistent state
    TaggedValue state;

    // Intra-program messages
    Dict messages;

    // List of values that are being passed from the EvalContext owner to the script.
    List argumentList;

    // Current execution stack
    int numFrames;
    Frame* stack;

    Term* currentTerm;

    EvalContext();
    ~EvalContext();

private:
    // Disabled calls
    EvalContext(EvalContext const&) {}
    EvalContext& operator=(EvalContext const&) { return *this; }
};

void eval_context_print_multiline(std::ostream& out, EvalContext* context);
void eval_context_setup_type(Type* type);

// Stack frames
Frame* get_frame(EvalContext* context, int depth);
Frame* get_frame_from_bottom(EvalContext* context, int index);
Frame* push_frame(EvalContext* context, Branch* branch, List* registers);
Frame* push_frame(EvalContext* context, Branch* branch);
void push_frame_with_inputs(EvalContext* context, Branch* branch, ListData* args);
void pop_frame(EvalContext* context);
void finish_frame(EvalContext* context);
Frame* top_frame(EvalContext* context);
void reset_stack(EvalContext* context);

// Pre-evaluation
void write_input_instruction(Term* caller, Term* input, TaggedValue* isn);

// Evaluate a single term. This is not usually called directly, it's called
// by the interpreter.
void evaluate_single_term(EvalContext* context, Term* term);

void evaluate_branch(EvalContext* context, Branch* branch);

void evaluate_save_locals(EvalContext* context, Branch* branch);

// Shorthand to call evaluate_branch with a new EvalContext:
void evaluate_branch(Branch* branch);

// Evaluate only the terms between 'start' and 'end'.
void evaluate_range(EvalContext* context, Branch* branch, int start, int end);

// Evaluate 'term' and every term that it depends on.
void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result);

// Parse input and immediately evaluate it, returning the result value.
TaggedValue* evaluate(EvalContext* context, Branch* branch, std::string const& input);
TaggedValue* evaluate(Branch* branch, Term* function, List* inputs);
TaggedValue* evaluate(Term* function, List* inputs);

TaggedValue* get_arg(EvalContext* context, ListData* args, int index);
TaggedValue* get_arg(EvalContext* context, TaggedValue* arg);
void consume_arg(EvalContext* context, ListData* args, int index, TaggedValue* dest);
TaggedValue* get_output(EvalContext* context, ListData* args);

Term* current_term(EvalContext* context);
TaggedValue* get_register(EvalContext* context, Term* term);

// Signal that a runtime error has occurred.
void error_occurred(EvalContext* context, Term* term, TaggedValue* output, const char* msg);
void error_occurred(EvalContext* context, const char* msg);
void error_occurred(EvalContext* context, std::string const& msg);

void print_runtime_error_formatted(EvalContext& context, std::ostream& output);

// Returns whether evaluation has been interrupted, such as with a 'return' or
// 'break' statement, or a runtime error.
bool evaluation_interrupted(EvalContext* context);

void clear_error(EvalContext* cxt);

std::string context_get_error_message(EvalContext* cxt);
void context_print_error_stack(std::ostream& out, EvalContext* cxt);

void run_interpreter(EvalContext* context);

} // namespace circa
