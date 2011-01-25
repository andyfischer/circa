// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

    // Persistent state that should be used for the current branch
    TaggedValue currentScopeState;

    // State used for the current for loop
    ForLoopContext forLoopContext;

    // Intra-program messages
    Dict messages;

    EvalContext() : interruptSubroutine(false), errorOccurred(false) {}

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

void evaluate_branch_internal_with_state(EvalContext* context, Term* term);

void evaluate_branch_no_preserve_locals(EvalContext* context, Branch& branch);

// Top-level call. Evalaute the branch and then preserve stack outputs back to terms.
void evaluate_branch(EvalContext* context, Branch& branch);

// Shorthand to call evaluate_branch with a new EvalContext:
void evaluate_branch(Branch& branch);

TaggedValue* get_input(EvalContext* cxt, Term* term, int index);
TaggedValue* get_output(EvalContext* cxt, Term* term);
TaggedValue* get_extra_output(Term* term, int index);
TaggedValue* get_state_input(EvalContext* cxt, Term* term);
TaggedValue* get_local(Term* term);
TaggedValue* get_local_safe(Term* term);
Dict* get_current_scope_state(EvalContext* cxt);
void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output);
void preserve_state_result(Term* term, TaggedValue* container, TaggedValue* result);

void evaluate_range(EvalContext* context, Branch& branch, int start, int end);

void start_using(Branch& branch);
void finish_using(Branch& branch);

void evaluate_minimum(EvalContext* context, Term* term);

} // namespace circa
