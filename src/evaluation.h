// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"
#include "for_loop.h"
#include "references.h"
#include "tagged_value.h"
#include "types/list.h"
#include "types/dict.h"

namespace circa {

struct EvalContext
{
    bool interruptSubroutine;

    TaggedValue subroutineOutput;

    // Error information:
    bool errorOccurred;
    Ref errorTerm;
    std::string errorMessage;

    // Tree of persistent state
    TaggedValue state;

    // Names of state variables which need to be preserved at the end of this branch
    List openStateVariables;

    // Persistent state that should be used for the current branch
    TaggedValue currentScopeState;

    // State used for the current for loop
    ForLoopContext forLoopContext;

    EvalContext() : interruptSubroutine(false), errorOccurred(false), currentScopeState(NULL) {}

    void clearError() {
        errorOccurred = false;
        errorTerm = NULL;
        errorMessage = "";
    }
};

// Evaluate a single term against the given stack. After this function, the caller
// must reestablish any internal pointers to the contents of 'stack'.
void evaluate_single_term(EvalContext* context, Term* term);

void evaluate_branch_internal(EvalContext* context, Branch& branch);
void evaluate_branch_internal(EvalContext* context, Branch& branch, TaggedValue* output);

void wrap_up_open_state_vars(EvalContext* context, Branch& branch);

// Top-level call. Evalaute the branch and then preserve stack outputs back to terms.
void evaluate_branch(EvalContext* context, Branch& branch);

// Shorthand to call evaluate_branch with a new EvalContext:
EvalContext evaluate_branch(Branch& branch);

TaggedValue* get_input(EvalContext* cxt, Term* term, int index);
TaggedValue* get_output(EvalContext* cxt, Term* term);
TaggedValue* get_state_input(EvalContext* cxt, Term* term);
TaggedValue* get_local(Term* term);
Dict* get_current_scope_state(EvalContext* cxt);
void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output);
void preserve_state_result(Term* term, TaggedValue* container, TaggedValue* result);

void evaluate_range(EvalContext* context, Branch& branch, int start, int end);

void start_using(Branch& branch);
void finish_using(Branch& branch);

} // namespace circa
