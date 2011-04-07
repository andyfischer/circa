// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "building.h"
#include "build_options.h"
#include "builtins.h"
#include "branch.h"
#include "branch_iterator.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "locals.h"
#include "parser.h"
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

    // For a test build, we check the type result of every single call. This is slow and
    // it should be unnecessary if the function is written correctly. But it's a good
    // test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred) {
        for (int i=0; i < get_output_count(term); i++) {

            Type* outputType = unbox_type(get_output_type(term, i));
            TaggedValue* output = get_output(term, i);

            // Special case, if the function's output type is void then we don't care
            // if the output value is null or not.
            if (i == 0 && outputType == &VOID_T)
                continue;

            std::stringstream msg;
            if (!cast_possible(output, outputType)) {
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

          if (evaluation_interrupted(context))
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

    swap(&context->currentScopeState, &context->state);
    set_null(&context->currentScopeState);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    evaluate_branch_no_preserve_locals(context, branch);

    // Copy stack back to the original terms. Many tests depend on this functionality.
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
    return get_local(input, term->inputInfo(index)->outputIndex);
}

void consume_input(Term* term, int index, TaggedValue* dest)
{
    Term* input = term->input(index);
    if (input == NULL) {
        set_null(dest);
        return;
    }
    TaggedValue* val = get_local(input, term->inputInfo(index)->outputIndex);

    // if this function is called, then users shouldn't be 0.
    ca_assert(input->users.length() != 0);

    if (input->users.length() == 1) {
        swap(val,dest);
        set_null(val);
    } else {
        copy(val,dest);
    }
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

    int index = term->localsIndex + outputIndex;
    ca_assert(index < term->owningBranch->locals.length());
    TaggedValue* local = term->owningBranch->locals[index];
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
    int index = term->localsIndex + outputIndex;
    if (index >= term->owningBranch->locals.length())
        return NULL;
    TaggedValue* local = term->owningBranch->locals[index];
    return local;
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
bool evaluation_interrupted(EvalContext* context)
{
    return context->errorOccurred || context->interruptSubroutine
        || context->forLoopContext.breakCalled || context->forLoopContext.continueCalled;
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
    if (branch.inuse)
    {
        swap(&branch.locals, branch.localsStack.append());
        set_list(&branch.locals, get_locals_count(branch));
    } else {
        branch.locals.resize(get_locals_count(branch));
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

    start_using(branch);

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

    finish_using(branch);
}

TaggedValue* evaluate(EvalContext* context, Branch& branch, std::string const& input)
{
    int prevHead = branch.length();
    Term* result = parser::compile(&branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch.length() - 1);
    return get_local(result);
}

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
    cxt->errorMessage = "";
}

} // namespace circa
