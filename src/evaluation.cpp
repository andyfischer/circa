// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "building.h"
#include "build_options.h"
#include "builtins.h"
#include "branch.h"
#include "branch_check_invariants.h"
#include "branch_iterator.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "types/dict.h"

namespace circa {

void evaluate_single_term(EvalContext* context, Term* term)
{
    EvaluateFunc func = function_t::get_evaluate(term->function);
    ca_assert(func != NULL);

    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    func(context, term);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return error_occurred(context, term, e.what()); }
    #endif

    #if CIRCA_ALWAYS_TYPE_CHECK_OUTPUTS
    {
        Type* outputType = unbox_type(term->type);
        TaggedValue* output = get_local(term);

        if (!context->errorOccurred && !value_fits_type(output, outputType)) {
            std::stringstream msg;
            msg << "Function " << term->function->name << " produced output "
                << output->toString() << " which doesn't fit output type "
                << outputType->name;
            internal_error(msg.str());
        }
    }
    #endif
}

void evaluate_branch_internal(EvalContext* context, Branch& branch)
{
    start_using(branch);

    for (int i=0; i < branch.length(); i++) {
        evaluate_single_term(context, branch[i]);

//        if (context->errorOccurred || context->interruptSubroutine)
//            break;
          if (context->errorOccurred)
              break;
    }

    finish_using(branch);
}

void evaluate_branch_internal(EvalContext* context, Branch& branch, TaggedValue* output)
{
    start_using(branch);

    for (int i=0; i < branch.length(); i++)
        evaluate_single_term(context, branch[i]);

    if (output != NULL)
        copy(get_local(branch[branch.length()-1]), output);

    finish_using(branch);
}

void evaluate_branch_internal_with_state(EvalContext* context, Term* term)
{
    Branch& contents = term->nestedContents;

    // Store currentScopeState and fetch the container for this branch
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);
    fetch_state_container(term, &prevScopeState, &context->currentScopeState);

    evaluate_branch_internal(context, contents);

    // Store container and replace currentScopeState
    preserve_state_result(term, &prevScopeState, &context->currentScopeState);
    swap(&context->currentScopeState, &prevScopeState);
}

void evaluate_branch_no_preserve_locals(EvalContext* context, Branch& branch)
{
    copy(&context->state, &context->currentScopeState);
    evaluate_branch_internal(context, branch);
    copy(&context->currentScopeState, &context->state);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    evaluate_branch_no_preserve_locals(context, branch);

    // Copy stack back to terms, for backwards compatibility
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (is_value(term)) continue;
        TaggedValue* val = get_local(term);
        if (val != NULL)
            copy(val, branch[i]);
    }
}

void evaluate_branch(Branch& branch)
{
    EvalContext context;
    evaluate_branch(&context, branch);
}

TaggedValue* get_input(EvalContext* cxt, Term* term, int index)
{
    Term* input = term->input(index);
    if (input == NULL)
        return NULL;
    return get_local(input);
}

TaggedValue* get_output(EvalContext* cxt, Term* term)
{
    return get_local(term);
}

TaggedValue* get_extra_output(Term* term, int index)
{
    return get_local(term->owningBranch->get(term->index + 1 + index));
}

TaggedValue* get_state_input(EvalContext* cxt, Term* term)
{
    if (term->input(0) == NULL) {
        Dict* currentScopeState = get_current_scope_state(cxt);
        ca_assert(currentScopeState != NULL);
        return currentScopeState->insert(term->uniqueName.name.c_str());
    } else {
        return get_input(cxt, term, 0);
    }
}

TaggedValue* get_local(Term* term)
{
    ca_assert(term->owningBranch != NULL);

    // TODO: make sure that this list is the right size when building
    // the branch, instead of here.

    term->owningBranch->locals.resize(term->owningBranch->length());

    TaggedValue* local = term->owningBranch->locals[term->index];
    return local;
}

TaggedValue* get_local_safe(Term* term)
{
    if (term->owningBranch == NULL)
        return NULL;
    return get_local(term);
}

Dict* get_current_scope_state(EvalContext* cxt)
{
    return Dict::lazyCast(&cxt->currentScopeState);
}
void fetch_state_container(Term* term, TaggedValue* container, TaggedValue* output)
{
    Dict* containerDict = Dict::lazyCast(container);
    copy(containerDict->insert(term->uniqueName.name.c_str()), output);
}
void preserve_state_result(Term* term, TaggedValue* container, TaggedValue* result)
{
    Dict* containerDict = Dict::lazyCast(container);
    const char* name = term->uniqueName.name.c_str();
    copy(result, containerDict->insert(name));
}

void evaluate_range(EvalContext* context, Branch& branch, int start, int end)
{
    branch.locals.resize(branch.length());
    for (int i=start; i <= end; i++)
        evaluate_single_term(context, branch[i]);

    // copy locals back to terms
    for (int i=start; i <= end; i++) {
        Term* term = branch[i];
        if (is_value(term))
            continue;
        TaggedValue* value = get_local(term);
        if (value == NULL)
            continue;
        copy(value, term);
    }
}

void start_using(Branch& branch)
{
    if (branch.inuse) {
        swap(&branch.locals, branch.localsStack.append());
        set_list(&branch.locals, branch.length());
    }

    branch.inuse = true;
}

void finish_using(Branch& branch)
{
    ca_assert(branch.inuse);

    if (branch.localsStack.length() == 0) {
        branch.inuse = false;
    } else {
        swap(&branch.locals, branch.localsStack.getLast());
        branch.localsStack.pop();
    }
}

void evaluate_minimum(EvalContext* context, Term* term)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch& branch = *term->owningBranch;

    bool *marked = new bool[branch.length()];
    memset(marked, false, sizeof(bool)*branch.length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        if (marked[i]) {
            for (int inputIndex=0; inputIndex < term->numInputs(); inputIndex++) {
                Term* input = term->input(inputIndex);
                if (input == NULL) continue;
                if (input->owningBranch != &branch) continue;
                marked[input->index] = true;
            }
        }
    }

    for (int i=0; i <= term->index; i++) {
        if (marked[i]) {
            evaluate_single_term(context, branch[i]);
        }
    }

    delete[] marked;
}

} // namespace circa
