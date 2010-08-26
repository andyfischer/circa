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

#define VERBOSE_PRINT_VM 0

namespace circa {

void evaluate_term(EvalContext* cxt, Term* caller, Term* function, RefList const& inputs, TaggedValue* output)
{
#ifndef BYTECODE
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
#endif
}

inline void evaluate_term(EvalContext* cxt, Term* term)
{
#ifndef BYTECODE
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
#endif
}

void evaluate_term(Term* term)
{
    EvalContext context;
    evaluate_term(&context, term);
}

void evaluate_branch(EvalContext* context, Branch& branch)
{
#ifdef BYTECODE
    bytecode::update_bytecode(branch);
    List stack;
    evaluate_bytecode(context, &branch._bytecode, &stack);
    copy_stack_back_to_terms(branch, &stack);
    
    reset(&context->subroutineOutput);
#else

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
#endif
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

void evaluate_bytecode(EvalContext* cxt, bytecode::BytecodeData* data, List* stack)
{
    //std::cout << "running bytecode:" << std::endl;
    //print_bytecode(std::cout, data);

    //cxt->clearError();
    data->inuse = true;

    char* pos = data->opdata;
    char* end = data->opdata + data->size;

    while (pos != end) {

        bytecode::Operation* op = (bytecode::Operation*) pos;

        #if VERBOSE_PRINT_VM
            std::cout << std::endl << "stack: " << stack->toString() << std::endl;
            std::cout << "state: " << cxt->topLevelState.toString() << std::endl;
            std::cout << "next op: ";
            print_operation(std::cout, op);
        #endif

        switch (op->opid) {
            case bytecode::OP_STACK_SIZE: {
                bytecode::StackSizeOperation *ssop = (bytecode::StackSizeOperation*) op;
                stack->resize(ssop->numElements);
                pos += sizeof(bytecode::StackSizeOperation);
                continue;
            }
            case bytecode::OP_PUSH_VALUE: {
                bytecode::PushValueOperation *callop = (bytecode::PushValueOperation*) op;
                TaggedValue* output = stack->get(callop->outputIndex);
                ca_assert(output != NULL);
                change_type(output, type_contents(callop->source->type));
                copy(callop->source, output);
                pos += sizeof(bytecode::PushValueOperation);
                continue;
            }
            case bytecode::OP_CALL: {
                bytecode::CallOperation *callop = (bytecode::CallOperation*) op;
                //std::cout << "calling: " << callop->function->name << std::endl;

                EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;
                func(cxt, stack, callop);

                #if 0
                // Temp: Assemble arguments for compatibility with old-style
                // evaluation func. This involves a bunch of wasteful copying
                // which will be removed later.
                RefList inputs;
                inputs.resize(callop->numInputs);
                for (int i=0; i < callop->numInputs; i++) {
                    inputs[i] = alloc_term();
                    TaggedValue* stackInput = NULL;
                    int stackIndex = callop->inputs[i].stackIndex;
                    if (stackIndex == -1) {
                        inputs[i] = NULL;
                    } else {
                        stackInput = stack->get(stackIndex);
                        change_type(inputs[i], stackInput->value_type);
                        copy(stackInput, inputs[i]);
                    }
                }
                TaggedValue* output = NULL;
                if (callop->outputIndex != -1) {
                    output = stack->get(callop->outputIndex);
                    Type* type = type_contents(function_t::get_output_type(callop->function));
                    change_type(output, type);
                }

                evaluate_term(cxt, callop->caller, callop->function, inputs, output);
                #endif

                pos += sizeof(bytecode::CallOperation)
                    + sizeof(bytecode::CallOperation::Input)*callop->numInputs;
                continue;
            }
            case bytecode::OP_JUMP: {
                bytecode::JumpOperation *jumpop = (bytecode::JumpOperation*) op;
                pos = data->opdata + jumpop->offset;
                continue;
            }
            case bytecode::OP_JUMP_IF: {
                bytecode::JumpIfOperation *jumpop = (bytecode::JumpIfOperation*) op;
                if (as_bool(stack->get(jumpop->conditionIndex)))
                    pos = data->opdata + jumpop->offset;
                else
                    pos += sizeof(bytecode::JumpIfOperation);
                continue;
            }
            case bytecode::OP_JUMP_IF_NOT: {
                bytecode::JumpIfNotOperation *jumpop = (bytecode::JumpIfNotOperation*) op;
                if (!as_bool(stack->get(jumpop->conditionIndex)))
                    pos = data->opdata + jumpop->offset;
                else
                    pos += sizeof(bytecode::JumpIfNotOperation);
                continue;
            }

            case bytecode::OP_RETURN: {
                bytecode::ReturnOperation *retop = (bytecode::ReturnOperation*) op;
                if (retop->stackIndex != -1) {
                    copy(stack->get(retop->stackIndex), &cxt->subroutineOutput);
                }
                goto loop_end;
            }
            case bytecode::OP_PUSH_INT: {
                bytecode::PushIntOperation *pushop = (bytecode::PushIntOperation*) op;
                make_int(stack->get(pushop->outputIndex), pushop->value);
                pos += sizeof(bytecode::PushIntOperation);
                continue;
            }
            case bytecode::OP_INCREMENT: {
                bytecode::IncrementOperation *incop = (bytecode::IncrementOperation*) op;
                TaggedValue* value = stack->get(incop->stackIndex);
                set_int(value, as_int(value) + 1);
                pos += sizeof(bytecode::IncrementOperation);
                continue;
            }
            case bytecode::OP_GET_INDEX: {
                bytecode::GetIndexOperation *getop = (bytecode::GetIndexOperation*) op;
                TaggedValue *list = stack->get(getop->listIndex);
                TaggedValue *listIndex = stack->get(getop->indexInList);
                TaggedValue *item = list->getIndex(as_int(listIndex));
                copy(item, stack->get(getop->outputIndex));
                pos += sizeof(bytecode::GetIndexOperation);
                continue;
            }
            case bytecode::OP_APPEND: {
                bytecode::AppendOperation *appendop = (bytecode::AppendOperation*) op;
                TaggedValue *item = stack->get(appendop->itemIndex);
                copy(item, ((List*) stack->get(appendop->outputIndex))->append());
                pos += sizeof(bytecode::AppendOperation);
                continue;
            }
            case bytecode::OP_NUM_ELEMENTS: {
                bytecode::NumElementsOperation *neop = (bytecode::NumElementsOperation*) op;
                TaggedValue *list = stack->get(neop->listIndex);
                make_int(stack->get(neop->outputIndex), list->numElements());
                pos += sizeof(bytecode::NumElementsOperation);
                continue;
            }
            case bytecode::OP_COPY: {
                bytecode::CopyOperation *copyop = (bytecode::CopyOperation*) op;
                copy(stack->get(copyop->fromIndex), stack->get(copyop->toIndex));
                pos += sizeof(bytecode::CopyOperation);
                continue;
            }
            case bytecode::OP_RAISE: {
                //TODO
                //bytecode::RaiseOperation *raiseop = (bytecode::RaiseOperation*) op;
                cxt->errorOccurred = true;
                pos += sizeof(bytecode::RaiseOperation);
                continue;
            }
            case bytecode::OP_CHECK_ERROR: {
                if (cxt->errorOccurred)
                    goto loop_end;
                pos += sizeof(bytecode::CheckErrorOperation);
                continue;
            }
            case bytecode::OP_COMMENT: {
                bytecode::CommentOperation *commentop = (bytecode::CommentOperation*) op;
                pos += commentop->size;
                continue;
            }
            case bytecode::OP_VAR_NAME: {
                bytecode::VarNameOperation *nameop = (bytecode::VarNameOperation*) op;
                pos += nameop->size;
                continue;
            }
        }
    }

loop_end:
    data->inuse = false;
    #if VERBOSE_PRINT_VM
        std::cout << std::endl << "stack: " << stack->toString() << std::endl;
        std::cout << "state: " << cxt->topLevelState.toString() << std::endl;
    #endif
}

void evaluate_bytecode(Branch& branch)
{
    EvalContext context;
    List stack;
    bytecode::update_bytecode(branch);
    evaluate_bytecode(&context, &branch._bytecode, &stack);
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

} // namespace circa
