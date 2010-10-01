// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "building.h"
#include "builtins.h"
#include "branch.h"
#include "branch_iterator.h"
#include "bytecode.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"

namespace circa {

void evaluate_single_term(EvalContext* context, List* stack, Term* term)
{
    EvaluateFunc func = function_t::get_evaluate(term->function);
    if (func != NULL) 
        func(context, stack, term);
}

void evaluate_branch_existing_frame(EvalContext* context, List* stack, Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        evaluate_single_term(context, stack, branch[i]);
}

void evaluate_branch(EvalContext* context, List* stack, Branch& branch, TaggedValue* output)
{
    List* frame = push_stack_frame(stack, branch.registerCount);
    evaluate_branch_existing_frame(context, stack, branch);
    frame = get_stack_frame(stack, 0);
    if (output != NULL) {
        TaggedValue* lastValue = frame->get(frame->length()-1);
        if (lastValue != NULL)
            swap(lastValue, output);
        else
            make_null(output);
    }
    pop_stack_frame(stack);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    List stack;
    push_stack_frame(&stack, branch.registerCount);
    evaluate_branch_existing_frame(context, &stack, branch);

    // Copy stack back to terms
    List* frame = get_stack_frame(&stack, 0);
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (is_value(term)) continue;
        if (term->registerIndex != -1)
            swap(frame->get(term->registerIndex), branch[i]);
    }
}

EvalContext evaluate_branch(Branch& branch)
{
    EvalContext context;
    evaluate_branch(&context, branch);
    return context;
}

Term* apply_and_eval(Branch& branch, Term* function, RefList const& inputs)
{
    Term* result = apply(branch, function, inputs);
    // TODO: eval
    return result;
}

Term* apply_and_eval(Branch& branch, std::string const& functionName,
        RefList const &inputs)
{
    Term* function = find_named(branch, functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    return apply_and_eval(branch, function, inputs);
}

void evaluate_bytecode(Branch& branch)
{
    EvalContext context;
    List stack;
    bytecode::update_bytecode(branch);
    bytecode::evaluate_bytecode(&context, &branch._bytecode, &stack);
}

void copy_stack_back_to_terms(Branch& branch, List* stack)
{
    for (BranchIterator it(branch); !it.finished(); ++it) {
        Term* term = *it;
        if (term->registerIndex == -1)
            continue;

        // Don't modify value terms.
        if (is_value(term))
            continue;

        TaggedValue* value = stack->get(term->registerIndex);
        if (value == NULL)
            continue;

        copy(value, term);
    }
}

void capture_inputs(List* stack, bytecode::CallOperation* callOp, List* inputs)
{
    touch(inputs);
    inputs->resize(callOp->numInputs);
    for (int i=0; i < callOp->numInputs; i++)
        copy(stack->get(callOp->inputs[i].registerIndex), inputs->get(i));
}

TaggedValue* get_input(List* stack, Term* term, int index)
{
    Term* input = term->input(index);
    InputInfo& inputInfo = term->inputInfo(index);

    if (input->registerIndex == -1)
        return NULL;

    List* frame = List::checkCast(stack->get(stack->length() - 1 - inputInfo.relativeScope));
    return frame->get(term->input(index)->registerIndex);
}

TaggedValue* get_output(List* stack, Term* term)
{
    if (term->registerIndex == -1)
        return NULL;
    List* frame = List::checkCast(stack->get(stack->length()-1));
    return frame->get(term->registerIndex);
}

List* push_stack_frame(List* stack, int size)
{
    int newStackLength = stack->length() + 1;
    stack->resize(newStackLength);

    List* frame = make_list(stack->get(newStackLength-1), size);
    return frame;
}

void pop_stack_frame(List* stack)
{
    stack->resize(stack->length() - 1);
}

List* get_stack_frame(List* stack, int relativeScope)
{
    return List::checkCast(stack->get(stack->length() - 1 - relativeScope));
}

void evaluate_with_lazy_stack(EvalContext* context, List* stack, Term* term)
{
    // Check each input.
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input == NULL)
            continue;

        int relativeScope = term->inputInfo(i).relativeScope;

        // Check to add more stack frames.
        while (stack->length() < (relativeScope + 1)) {
            ca_assert(stack->length() < 1000); // <-- trying to catch a bug
            stack->prepend();
        }

        // Check to expand the stack frame that this input uses.
        TaggedValue* frameTv = stack->get(stack->length() - 1 -relativeScope);
        List* frame = List::checkCast(frameTv);
        int owningBranchRegCount = input->owningBranch->registerCount;

        if (frame == NULL)
            frame = make_list(frameTv, owningBranchRegCount);
        else
            frame->resize(owningBranchRegCount);

        // Check to populate this input's register
        if (is_null(frame->get(input->registerIndex)))
            copy(input, frame->get(input->registerIndex));

            ca_assert(stack->length() < 1000); // <-- trying to catch a bug
    }

    // Check that the stack has room for the term's output.
    if (stack->length() == 0)
        make_list(stack->append(), term->owningBranch->registerCount);
    else {
        List* frame = List::checkCast(stack->get(stack->length() - 1));
        frame->resize(term->owningBranch->registerCount);
    }

    // Evaluate
    evaluate_single_term(context, stack, term);

    // Copy output value back to term
    if (term->registerIndex != -1) {
        List* frame = List::checkCast(stack->get(stack->length() - 1));
        copy(frame->get(term->registerIndex), term);
    }

    ca_assert(stack->length() < 1000); // <-- trying to catch a bug
}

void evaluate_range_with_lazy_stack(EvalContext* context, Branch& branch, int start, int end)
{
    List stack;
    for (int i=start; i <= end; i++)
        evaluate_with_lazy_stack(context, &stack, branch[i]);
}

} // namespace circa
