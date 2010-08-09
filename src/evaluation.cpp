// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "building.h"
#include "builtins.h"
#include "branch.h"
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
    EvaluateFunc evaluate = function_t::get_evaluate(function);

    if (evaluate == NULL)
        return;

    try {
        evaluate(cxt, caller, function, inputs, output);
    }
    catch (std::exception const& err)
    {
        error_occurred(cxt, caller, err.what());
    }
}

inline void evaluate_term(EvalContext* cxt, Term* term)
{
    ca_assert(cxt != NULL);
    ca_assert(term != NULL);

    EvaluateFunc evaluate = function_t::get_evaluate(term->function);

    if (evaluate == NULL)
        return;

    // Execute the function
    try {
        evaluate(cxt, term, term->function, term->inputs, term);
    }
    catch (std::exception const& err)
    {
        error_occurred(cxt, term, err.what());
    }
}

void evaluate_term(Term* term)
{
    EvalContext context;
    evaluate_term(&context, term);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
    ca_assert(context != NULL);

    for (int index=0; index < branch.length(); index++) {
		Term* term = branch.get(index);
        evaluate_term(context, term);

        if (context->errorOccurred || context->interruptSubroutine)
            break;
    }

    // Copy the results of state vars
    for (int index=0; index < branch.length(); index++) {
        Term* term = branch[index];
        if (is_stateful(term) && term->input(0) != NULL)
            copy(term->input(0), term);
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

bool has_been_evaluated(Term* term)
{
    // TODO
    return true;
}

void evaluate_bytecode(EvalContext* cxt, bytecode::BytecodeData* data, List* stack)
{
    char* pos = data->opdata;
    char* end = data->opdata + data->size;

    while (pos != end) {
        bytecode::Operation* op = (bytecode::Operation*) pos;

        // todo: this could be inlined into the switch statement below
        pos += bytecode::get_operation_size(op);

        switch (op->opid) {
            case bytecode::OP_STACK_SIZE: {
                bytecode::StackSizeOperation *ssop = (bytecode::StackSizeOperation*) op;
                stack->resize(ssop->numElements);
                continue;
            }

            case bytecode::OP_RETURN: {
                // todo
                return;
            }

            case bytecode::OP_CALL: {
                bytecode::CallOperation *callop = (bytecode::CallOperation*) op;

                // Temp: Assemble arguments for compatibility with old-style
                // evaluation func. This involves a bunch of wasteful copying
                // which will be removed later.
                RefList inputs;
                inputs.resize(callop->numInputs);
                for (int i=0; i < callop->numInputs; i++) {
                    inputs[i] = alloc_term();
                    TaggedValue* stackInput = stack->get(callop->inputs[i].stackIndex);
                    change_type(inputs[i], stackInput->value_type);
                    copy(stackInput, inputs[i]);
                }
                TaggedValue* output = NULL;
                if (callop->outputIndex != -1) {
                    output = stack->get(callop->outputIndex);
                    change_type(output, type_contents(callop->caller->type));
                }

                evaluate_term(cxt, callop->caller, callop->function, inputs, output);
                continue;
            }

            case bytecode::OP_PUSH_VALUE: {
                bytecode::PushValueOperation *callop = (bytecode::PushValueOperation*) op;

                TaggedValue* output = stack->get(callop->outputIndex);
                change_type(output, type_contents(callop->source->type));
                copy(callop->source, output);
                continue;
            }

        }
    }
}

} // namespace circa
