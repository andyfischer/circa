// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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
#include "locals.h"
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
    if (!context->errorOccurred) {
        for (int i=0; i < get_output_count(term); i++) {

            // even more temporary:
            if (i > 0)
                break;

            // temp exception:
            if (i > 0 && (term->function == FOR_FUNC || term->function == IF_BLOCK_FUNC))
                continue;

            Type* outputType = unbox_type(get_output_type(term, i));
            TaggedValue* output = get_output(term, i);

            std::stringstream msg;
            if (!value_fits_type(output, outputType)) {
                msg << "Function " << term->function->name << " produced output "
                    << output->toString() << " (in index " << i << ")"
                    << " which doesn't fit output type "
                    << outputType->name;
                internal_error(msg.str());
            }
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

TaggedValue* get_input(Term* term, int index)
{
    Term* input = term->input(index);
    if (input == NULL)
        return NULL;
    return get_local(input, term->inputs[index].outputIndex);
}

TaggedValue* get_output(Term* term, int outputIndex)
{
    return get_local(term, outputIndex);
}

TaggedValue* get_extra_output(Term* term, int index)
{
    return get_output(term, index + 1);
}

TaggedValue* get_state_input(EvalContext* cxt, Term* term)
{
    if (term->input(0) == NULL) {
        Dict* currentScopeState = get_current_scope_state(cxt);
        ca_assert(currentScopeState != NULL);
        return currentScopeState->insert(term->uniqueName.name.c_str());
    } else {
        return get_input(term, 0);
    }
}

TaggedValue* get_local(Term* term, int outputIndex)
{
    ca_assert(term->owningBranch != NULL);

    // Make sure Branch.locals has the right size.
    //
    // TODO: Do this check earlier instead of here.

    term->owningBranch->locals.resize(term->owningBranch->localsCount);

    TaggedValue* local = term->owningBranch->locals[term->localsIndex + outputIndex];
    return local;
}

TaggedValue* get_local(Term* term)
{
    return get_local(term, 0);
}

TaggedValue* get_local_safe(Term* term, int outputIndex)
{
    if (term->owningBranch == NULL)
        return NULL;
    return get_local(term, outputIndex);
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

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
    cxt->errorMessage = "";
}

} // namespace circa
