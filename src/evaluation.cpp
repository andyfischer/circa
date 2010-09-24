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

void evaluate_branch(EvalContext* context, List *stack, Branch& branch, TaggedValue* output)
{
    List* frame = push_stack_frame(stack);
    frame->resize(branch.length());

    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        EvaluateFunc func = function_t::get_evaluate(term->function);
        if (func != NULL) 
            func(context, stack, term);
    }

    // Save output value
    if (output != NULL)
        swap(frame->get(frame->length()-1), output);

    pop_stack_frame(stack);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    List stack;
    stack.resize(1);
    evaluate_branch(context, &stack, branch, NULL);
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
    InputInfo& input = term->inputInfo(index);
    Term* inputTerm = term->input(index);

    List* registers = List::checkCast(stack->get(stack->length() - 1 - input.relativeScope));

    return registers->get(inputTerm->index);
}

TaggedValue* get_output(List* stack, Term* term)
{
    List* registers = List::checkCast(stack->get(stack->length()-1));

    return registers->get(term->index);
}

void evaluate_single_term(Term* caller)
{
#ifdef BYTECODE
    int numInputs = caller->numInputs();

    List registers;
    registers.resize(numInputs + 1);

    // Populate registers with inputs
    for (int i=0; i < caller->numInputs(); i++)
        copy(caller->input(i), registers.get(i));

    bytecode::CallOperation *callop = (bytecode::CallOperation*)
        malloc(sizeof(bytecode::CallOperation)
                + numInputs*sizeof(bytecode::CallOperation::Input));

    callop->caller = caller;
    callop->function = caller->function;
    callop->numInputs = numInputs;

    for (int i=0; i < caller->numInputs(); i++)
        callop->inputs[i].registerIndex = i;

    callop->outputIndex = numInputs;

    EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;

    EvalContext context;
    func(&context, &registers, callop);

    delete callop;

    // Copy output
    copy(registers.get(numInputs), caller);
#endif
}

List* push_stack_frame(List* stack)
{
    int newStackLength = stack->length() + 1;
    stack->resize(newStackLength);

    List* frame = make_list(stack->get(newStackLength-1));
    return frame;
}

void pop_stack_frame(List* stack)
{
    stack->resize(stack->length() - 1);
}

#ifdef BYTECODE
void evaluate_single_call_op(EvalContext *cxt, bytecode::CallOperation* callop,
    List* registers)
{
    EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;
    func(cxt, registers, callop);
}
#endif

} // namespace circa
