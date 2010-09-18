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

void evaluate_term(EvalContext* cxt, Term* caller, Term* function, RefList const& inputs, TaggedValue* output)
{
}

inline void evaluate_term(EvalContext* cxt, Term* term)
{
}

void evaluate_term(Term* term)
{
    EvalContext context;
    evaluate_term(&context, term);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    bytecode::update_bytecode(branch);
    List stack;
    bytecode::evaluate_bytecode(context, &branch._bytecode, &stack);
    copy_stack_back_to_terms(branch, &stack);
    
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
    evaluate_term(result);
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

void evaluate_without_side_effects(Term* term)
{
    // TODO: Should actually check if the function has side effects.
    for (int i=0; i < term->numInputs(); i++) {
        Term* input = term->input(i);
        if (input->owningBranch == term->owningBranch)
            evaluate_without_side_effects(input);
    }

    evaluate_term(term);
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
        if (term->stackIndex == -1)
            continue;

        // Don't modify value terms
        if (is_value(term))
            continue;

        TaggedValue* value = stack->get(term->stackIndex);
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
        copy(stack->get(callOp->inputs[i].stackIndex), inputs->get(i));
}

TaggedValue* get_input(List* stack, bytecode::CallOperation* callOp, int index)
{
    ca_assert(index < callOp->numInputs);
    int stackIndex = callOp->inputs[index].stackIndex;
    if (stackIndex == -1)
        return NULL;
    return stack->get(stackIndex);
}

TaggedValue* get_output(List* stack, bytecode::CallOperation* callOp)
{
    if (callOp->outputIndex == -1)
        return NULL;
    return stack->get(callOp->outputIndex);
}

void evaluate_single_term(EvalContext* context, Term* caller, Term* function,
        List* inputs, TaggedValue* output)
{
    bytecode::CallOperation callop;
    callop.caller = caller;
    callop.function = function;

    std::cout << "evaluate_single_term" << std::endl;
}

void evaluate_single_call_op(EvalContext *cxt, bytecode::CallOperation* callop, List* stack)
{
    EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;
    func(cxt, stack, callop);
}

} // namespace circa
