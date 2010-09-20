// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "building.h"
#include "builtins.h"
#include "branch.h"
#include "branch_iterator.h"
#include "bytecode.h"
#include "errors.h"
#include "evaluation.h"
#include "function.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"


namespace circa {

void evaluate_branch(EvalContext* context, Branch& branch)
{
    bytecode::update_bytecode(branch);
    List registers;
    bytecode::evaluate_bytecode(context, &branch._bytecode, &registers);
    copy_stack_back_to_terms(branch, &registers);
    
    reset(&context->subroutineOutput);
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

        // Don't modify value terms
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

TaggedValue* get_input(List* stack, bytecode::CallOperation* callOp, int index)
{
    ca_assert(index < callOp->numInputs);
    int registerIndex = callOp->inputs[index].registerIndex;
    if (registerIndex == -1)
        return NULL;
    return stack->get(registerIndex);
}

TaggedValue* get_output(List* stack, bytecode::CallOperation* callOp)
{
    if (callOp->outputIndex == -1)
        return NULL;
    return stack->get(callOp->outputIndex);
}

void evaluate_single_term(Term* caller)
{
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
}

void evaluate_single_call_op(EvalContext *cxt, bytecode::CallOperation* callop,
    List* registers)
{
    EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;
    func(cxt, registers, callop);
}


} // namespace circa
