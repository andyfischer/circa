// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "building.h"
#include "build_options.h"
#include "builtins.h"
#include "branch.h"
#include "code_iterators.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "list_shared.h"
#include "locals.h"
#include "parser.h"
#include "refactoring.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"
#include "types/dict.h"

namespace circa {

void evaluate_single_term(EvalContext* context, Term* term)
{
    #if CIRCA_THROW_ON_ERROR
    try {
    #endif

    term->evaluateFunc(context, term);

    #if CIRCA_THROW_ON_ERROR
    } catch (std::exception const& e) { return error_occurred(context, term, e.what()); }
    #endif

    // For a test build, we check the type of the output of every single call. This is
    // slow, and it should be unnecessary if the function is written correctly. But it's
    // a good test.
    #ifdef CIRCA_TEST_BUILD
    if (!context->errorOccurred && !is_value(term)) {
        for (int i=0; i < get_output_count(term); i++) {

            Type* outputType = get_output_type(term, i);
            TaggedValue* output = get_output(context, term, i);

            // Special case, if the function's output type is void then we don't care
            // if the output value is null or not.
            if (i == 0 && outputType == &VOID_T)
                continue;

            if (!cast_possible(output, outputType)) {
                std::stringstream msg;
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

void evaluate_branch_internal_with_state(EvalContext* context, Term* term,
        Branch& branch)
{
    // Store currentScopeState and fetch the container for this branch
    TaggedValue prevScopeState;
    swap(&context->currentScopeState, &prevScopeState);
    fetch_state_container(term, &prevScopeState, &context->currentScopeState);

    evaluate_branch_internal(context, branch);

    // Store container and replace currentScopeState
    save_and_consume_state(term, &prevScopeState, &context->currentScopeState);
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

TaggedValue* get_input(EvalContext* context, Term* term, int index)
{
    InputInstruction *instruction = &term->inputIsns.inputs[index];

    switch (instruction->type) {
    case InputInstruction::GLOBAL:
        return (TaggedValue*) term->input(index);
    case InputInstruction::OLD_STYLE_LOCAL:
        return get_local(term->input(index), term->inputInfo(index)->outputIndex);
    case InputInstruction::EMPTY:
        return NULL;
    case InputInstruction::LOCAL: {
        TaggedValue* frame = list_get_index_from_end(&context->stack,
            instruction->data.relativeFrame);
        return list_get_index(frame, instruction->data.index);
    }
    case InputInstruction::LOCAL_CONSUME:
    default:
        internal_error("Not yet implemented");
        return NULL;
    }
}

void consume_input(EvalContext* context, Term* term, int index, TaggedValue* dest)
{
    // Temp, don't actually consume
    copy(get_input(context, term, index), dest);
}

TaggedValue* get_output(EvalContext* context, Term* term, int index)
{
    InputInstruction *instruction = &term->inputIsns.outputs[index];

    switch (instruction->type) {
    case InputInstruction::GLOBAL:
        return (TaggedValue*) term;
    case InputInstruction::OLD_STYLE_LOCAL:
        return get_local(term, index);
    case InputInstruction::EMPTY:
        internal_error("Attempt to access NULL output");
        return NULL;
    case InputInstruction::LOCAL: {
        TaggedValue* frame = list_get_index_from_end(&context->stack,
            instruction->data.relativeFrame);
        return list_get_index(frame, instruction->data.index);
    }
    case InputInstruction::LOCAL_CONSUME:
    default:
        internal_error("Invalid instruction in get_output: LOCAL_CONSUME");
        return NULL;
    }
}

TaggedValue* get_extra_output(EvalContext* context, Term* term, int index)
{
    return get_output(context, term, index + 1);
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

TaggedValue* get_local(Term* term, int outputIndex)
{
    //ca_assert(!is_value(term));

    ca_assert(term->owningBranch != NULL);

    int index = term->localsIndex + outputIndex;

    ca_assert(index < term->owningBranch->locals.length());

    return term->owningBranch->locals[index];
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

void error_occurred(EvalContext* context, Term* errorTerm, std::string const& message)
{
    // Save the error as this term's output value.
    TaggedValue* out = get_output(context, errorTerm, 0);
    set_string(out, message);
    out->value_type = &ERROR_T;

    // Check if there is an errored() call listening to this term. If so, then
    // continue execution.
    if (has_an_error_listener(errorTerm))
        return;

    if (DEBUG_TRAP_ERROR_OCCURRED)
        ca_assert(false);

    ca_assert(errorTerm != NULL);

    if (context == NULL)
        throw std::runtime_error(message);

    if (!context->errorOccurred) {
        context->errorOccurred = true;
        context->errorTerm = errorTerm;
    }
}
void print_runtime_error_formatted(EvalContext& context, std::ostream& output)
{
    output << get_short_location(context.errorTerm)
        << " " << context_get_error_message(&context);
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

void save_and_consume_state(Term* term, TaggedValue* container, TaggedValue* result)
{
    Dict* containerDict = Dict::lazyCast(container);
    const char* name = term->uniqueName.name.c_str();
    swap(result, containerDict->insert(name));
    set_null(result);
}

bool evaluation_interrupted(EvalContext* context)
{
    return context->errorOccurred || context->interruptSubroutine
        || context->forLoopContext.breakCalled || context->forLoopContext.continueCalled;
}

void evaluate_range(EvalContext* context, Branch& branch, int start, int end)
{
    start_using(branch);

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

    finish_using(branch);
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

void evaluate_minimum(EvalContext* context, Term* term, TaggedValue* result)
{
    // Get a list of every term that this term depends on. Also, limit this
    // search to terms inside the current branch.
    
    Branch& branch = *term->owningBranch;

    start_using(branch);

    bool *marked = new bool[branch.length()];
    memset(marked, false, sizeof(bool)*branch.length());

    marked[term->index] = true;

    for (int i=term->index; i >= 0; i--) {
        Term* checkTerm = branch[i];
        if (checkTerm == NULL)
            continue;

        if (marked[i]) {
            for (int inputIndex=0; inputIndex < checkTerm->numInputs(); inputIndex++) {
                Term* input = checkTerm->input(inputIndex);
                if (input == NULL)
                    continue;
                if (input->owningBranch != &branch)
                    continue;
                // don't follow :meta inputs
                if (function_get_input_meta(get_function_attrs(checkTerm->function),
                            inputIndex))
                    continue;
                marked[input->index] = true;
            }
        }
    }

    for (int i=0; i <= term->index; i++) {
        if (marked[i])
            evaluate_single_term(context, branch[i]);
    }

    // Possibly save output
    if (result != NULL)
        copy(get_local(term), result);

    delete[] marked;

    finish_using(branch);
}

TaggedValue* evaluate(EvalContext* context, Branch& branch, std::string const& input)
{
    int prevHead = branch.length();
    Term* result = parser::compile(branch, parser::statement_list, input);
    evaluate_range(context, branch, prevHead, branch.length() - 1);
    return get_local(result);
}

TaggedValue* evaluate(Branch& branch, Term* function, List* inputs)
{
    EvalContext context;

    TermList inputTerms;
    inputTerms.resize(inputs->length());

    for (int i=0; i < inputs->length(); i++)
        inputTerms.setAt(i, create_value(branch, inputs->get(i)));

    int prevHead = branch.length();
    Term* result = apply(branch, function, inputTerms);
    evaluate_range(&context, branch, prevHead, branch.length() - 1);
    return get_local(result);
}

TaggedValue* evaluate(Term* function, List* inputs)
{
    Branch scratch;
    return evaluate(scratch, function, inputs);
}

void clear_error(EvalContext* cxt)
{
    cxt->errorOccurred = false;
    cxt->errorTerm = NULL;
}

void reset_locals(Branch& branch)
{
    ca_assert(!branch.inuse);
    reset(&branch.locals);
    reset(&branch.localsStack);

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (term->nestedContents)
            reset_locals(nested_contents(term));
    }
}

std::string context_get_error_message(EvalContext* cxt)
{
    ca_assert(cxt != NULL);
    ca_assert(cxt->errorTerm != NULL);
    TaggedValue* output = get_output(cxt, cxt->errorTerm, 0);
    if (output == NULL)
        return "error message unavailable";
    return as_string(output);
}

} // namespace circa
