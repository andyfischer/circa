// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"
#include "references.h"
#include "tagged_value.h"

namespace circa {

struct EvalContext
{
    bool interruptSubroutine;

    TaggedValue subroutineOutput;

    // Error information:
    bool errorOccurred;
    Ref errorTerm;
    std::string errorMessage;

    // Top-level inlined state
    TaggedValue topLevelState;

    EvalContext() : interruptSubroutine(false), errorOccurred(false) {}

    void clearError() {
        errorOccurred = false;
        errorTerm = NULL;
        errorMessage = "";
    }

};

// Evaluate a single term against the given stack. After this function, the caller
// must reestablish any internal pointers to the contents of 'stack'.
void evaluate_single_term(EvalContext* context, List* stack, Term* term);

void evaluate_branch_existing_frame(EvalContext* context, List* stack, Branch& branch);

// Evaluate a branch with an existing EvalContext, stack, and branch. 'output' can be
// null, if it's not null then we'll copy the output register of this branch to it.
// When specifying 'output', don't use a value that is in the stack, because that
// seems to break things. Instead use a temporary.
void evaluate_branch(EvalContext* context, List *stack, Branch& branch, TaggedValue* output);

void evaluate_branch(EvalContext* context, Branch& branch);
void evaluate_bytecode(Branch& branch);

// Shorthand to call evaluate_branch with a new EvalContext:
EvalContext evaluate_branch(Branch& branch);

// Perform 'apply' and then evaluate the result.
Term* apply_and_eval(Branch& branch, Term* function, RefList const& inputs);

// Find the named function in this branch, and then do the above 'apply_and_eval'.
Term* apply_and_eval(Branch& branch,
                    std::string const& functionName,
                    RefList const& inputs);

void copy_stack_back_to_terms(Branch& branch, List* stack);
void capture_inputs(List* stack, bytecode::CallOperation* callOp, List* inputs);
TaggedValue* get_input(List* stack, Term* term, int index);
TaggedValue* get_output(List* stack, Term* term);

List* push_stack_frame(List* stack, int size);
void pop_stack_frame(List* stack);
List* get_stack_frame(List* stack, int relativeScope);

// Before evaluating the term, we'll check each input to see if it's not-null on
// the stack (or if the stack frame is missing). Any missing values will get
// copied from their respective terms.
void evaluate_with_lazy_stack(EvalContext* context, List* stack, Term* term);

void evaluate_range_with_lazy_stack(EvalContext* context, Branch& branch, int start, int end);

} // namespace circa
