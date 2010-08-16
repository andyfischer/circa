// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch_iterator.h"
#include "builtins.h"
#include "bytecode.h"
#include "debug.h"
#include "function.h"
#include "if_block.h"
#include "introspection.h"
#include "for_loop.h"
#include "stateful_code.h"
#include "term.h"

namespace circa {
namespace bytecode {

size_t get_operation_size(Operation* op)
{
    switch (op->opid) {
        case OP_STACK_SIZE: return sizeof(StackSizeOperation);
        case OP_CALL: {
            CallOperation *callOp = (CallOperation*) op;
            return sizeof(CallOperation) + sizeof(CallOperation::Input)*callOp->numInputs;
        }
        case OP_PUSH_VALUE: return sizeof(PushValueOperation);
        case OP_JUMP: return sizeof(JumpOperation);
        case OP_JUMP_IF: return sizeof(JumpIfOperation);
        case OP_JUMP_IF_NOT: return sizeof(JumpIfNotOperation);
        case OP_RETURN: return sizeof(ReturnOperation);
        case OP_PUSH_INT: return sizeof(PushIntOperation);
        case OP_INCREMENT: return sizeof(IncrementOperation);
        case OP_GET_INDEX: return sizeof(GetIndexOperation);
        case OP_APPEND: return sizeof(AppendOperation);
        case OP_NUM_ELEMENTS: return sizeof(NumElementsOperation);
        case OP_COPY: return sizeof(CopyOperation);
    }
    ca_assert(false);
    return 0;
}

bool should_term_output_go_on_stack(Term* term)
{
    if (term->type == VOID_TYPE)
        return false;

    // Other stuff to check here?
    
    return true;
}

bool should_term_generate_call(Term* term)
{
    if (term->function == COMMENT_FUNC)
        return false;

    return true;
}

void write_stack_size_op(WriteContext* context, int stacksize)
{
    if (context->writePos) {
        StackSizeOperation* op = (StackSizeOperation*) context->writePos;
        op->opid = OP_STACK_SIZE;
        op->numElements = stacksize;
    }
    context->advance(sizeof(StackSizeOperation));
}

void write_call_op(WriteContext* context, Term* caller, Term* function, int numInputs, int* inputIndexes, int outputIndex)
{
    if (context->writePos) {
        CallOperation* op = (CallOperation*) context->writePos;
        op->opid = OP_CALL;
        op->caller = caller;
        op->function = function;
        op->numInputs = numInputs;
        op->outputIndex = outputIndex;

        for (int i=0; i < numInputs; i++)
            op->inputs[i].stackIndex = inputIndexes[i];
    }
    context->advance(sizeof(CallOperation) + sizeof(CallOperation::Input)*numInputs);
}

void write_call_op(WriteContext* context, Term* term)
{
    int numInputs = term->numInputs();

    if (should_term_output_go_on_stack(term) && term->stackIndex == -1) {

        term->stackIndex = context->nextStackIndex++;
    }

    const int MAX_INPUTS_EVER = 1000;
    int inputIndexes[MAX_INPUTS_EVER];

    for (int i=0; i < numInputs; i++) {
        Term* input = term->input(i);
        if (input == NULL)
            inputIndexes[i] = -1;
        else
            inputIndexes[i] = input->stackIndex;
    }

    write_call_op(context, term, term->function, numInputs, inputIndexes, term->stackIndex);
}

void write_push_value_op(WriteContext* context, Term* term)
{
    if (context && term->stackIndex == -1) {
        ca_assert(term->function->name != "trace");
        term->stackIndex = context->nextStackIndex++;
    }

    if (context->writePos) {
        PushValueOperation* op = (PushValueOperation*) context->writePos;
        op->opid = OP_PUSH_VALUE;
        op->outputIndex = term->stackIndex;
        op->source = term;
    }
    context->advance(sizeof(PushValueOperation));
}

void write_jump(WriteContext* context, int offset)
{
    if (context->writePos) {
        JumpOperation* op = (JumpOperation*) context->writePos;
        op->opid = OP_JUMP;
        op->offset = offset;
    }
    context->advance(sizeof(JumpOperation));
}

void write_jump_if(WriteContext* context, int conditionIndex, int offset)
{
    if (context->writePos) {
        JumpIfOperation* op = (JumpIfOperation*) context->writePos;
        op->opid = OP_JUMP_IF;
        op->conditionIndex = conditionIndex;
        op->offset = offset;
    }
    context->advance(sizeof(JumpIfOperation));
}
void write_jump_if_not(WriteContext* context, int conditionIndex, int offset)
{
    if (context->writePos) {
        JumpIfNotOperation* op = (JumpIfNotOperation*) context->writePos;
        op->opid = OP_JUMP_IF_NOT;
        op->conditionIndex = conditionIndex;
        op->offset = offset;
    }
    context->advance(sizeof(JumpIfNotOperation));
}
void write_push_int(WriteContext* context, int value, int outputIndex)
{
    if (context->writePos) {
        PushIntOperation* op = (PushIntOperation*) context->writePos;
        op->opid = OP_PUSH_INT;
        op->value = value;
        op->outputIndex = outputIndex;
    }
    context->advance(sizeof(PushIntOperation));
}
void write_get_index(WriteContext* context, int listIndex, int indexInList, int outputIndex)
{
    if (context->writePos) {
        GetIndexOperation* op = (GetIndexOperation*) context->writePos;
        op->opid = OP_GET_INDEX;
        op->listIndex = listIndex;
        op->indexInList = indexInList;
        op->outputIndex = outputIndex;
    }
    context->advance(sizeof(GetIndexOperation));
}
void write_increment(WriteContext* context, int intIndex)
{
    if (context->writePos) {
        IncrementOperation* op = (IncrementOperation*) context->writePos;
        op->opid = OP_INCREMENT;
        op->stackIndex = intIndex;
    }
    context->advance(sizeof(IncrementOperation));
}
void write_num_elements(WriteContext* context, int listIndex, int outputIndex)
{
    if (context->writePos) {
        NumElementsOperation* op = (NumElementsOperation*) context->writePos;
        op->opid = OP_NUM_ELEMENTS;
        op->listIndex = listIndex;
        op->outputIndex = outputIndex;
    }
    context->advance(sizeof(NumElementsOperation));
}

void write_copy(WriteContext* context, int fromIndex, int toIndex)
{
    if (context->writePos) {
        CopyOperation* op = (CopyOperation*) context->writePos;
        op->opid = OP_COPY;
        op->fromIndex = fromIndex;
        op->toIndex = toIndex;
    }
    context->advance(sizeof(CopyOperation));
}

void write_return_op(WriteContext* context, Term* term)
{
    if (context->writePos) {
        ReturnOperation* op = (ReturnOperation*) context->writePos;
        op->opid = OP_RETURN;
        op->caller = term;
    }
    context->advance(sizeof(ReturnOperation));
}

void write_op(WriteContext* context, Term* term)
{
    if (!should_term_generate_call(term))
        return;
    if (term->function == RETURN_FUNC)
        return write_return_op(context, term);

    if (term->function == VALUE_FUNC)
        return write_push_value_op(context, term);

    if (term->function == IF_BLOCK_FUNC)
        return write_if_block_bytecode(context, term);

    if (term->function == FOR_FUNC)
        return write_for_loop_bytecode(context, term);

    FunctionAttrs::WriteBytecode writeBytecode
        = function_t::get_attrs(term->function).writeBytecode;
    if (writeBytecode)
        return writeBytecode(context, term);

    return write_call_op(context, term);
}

void write_bytecode_for_branch(WriteContext* context, Branch& branch, int inlineState,
        int firstIndex, int lastIndex)
{
    int prevInlineState = context->inlineState;

    context->inlineState = inlineState;

    if (lastIndex == -1)
        lastIndex = branch.length();

    for (int i=firstIndex; i < lastIndex; i++)
        write_op(context, branch[i]);

    // Wrap up any state vars that were declared in this branch.
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (is_stateful(term)) {
            Term* modifiedResult = branch[term->name];
            ca_assert(term->name != "");
            ca_assert(modifiedResult != NULL);
            ca_assert(modifiedResult->stackIndex != -1);
            int inputs[] = { context->inlineState, term->input(1)->stackIndex,
                modifiedResult->stackIndex };
            write_call_op(context, NULL, get_global("set_state_field"), 3, inputs,
                    context->inlineState);
        }
    }

    context->inlineState = prevInlineState;
}

void write_bytecode_for_top_level_branch(WriteContext* context, Branch& branch)
{
    // Clear stack indexes for all terms, these will be reassigned inside write_op.
    // Don't bother doing this if we aren't yet writing data.
    if (context->writePos) 
        for (BranchIterator it(branch); !it.finished(); ++it)
            it->stackIndex = -1;

    // First add a STACK_SIZE op. Will fill in the actual size later.
    StackSizeOperation *stackSizeOperation = (StackSizeOperation*) context->writePos;
    write_stack_size_op(context, 0);

    // If this branch has any state, then create a get_top_level_state call.
    bool hasAnyInlinedState = false;
    if (has_any_inlined_state(branch)) {
        hasAnyInlinedState = true;
        context->topLevelState = context->nextStackIndex++;
        write_call_op(context, NULL, get_global("get_top_level_state"), 0, NULL,
                context->topLevelState);
    }

    write_bytecode_for_branch(context, branch, context->topLevelState);

    if (hasAnyInlinedState) {
        int inputs[] = { context->topLevelState };
        write_call_op(context, NULL, get_global("set_top_level_state"), 1, inputs, -1);
    }
       
    if (stackSizeOperation)
        stackSizeOperation->numElements = context->nextStackIndex;
}

void update_bytecode(Branch& branch, BytecodeData* bytecode)
{
    // First pass: Figure out the size of generated bytecode.
    WriteContext context;
    context.writePos = NULL;
    context.sizeWritten = 0;

    write_bytecode_for_top_level_branch(&context, branch);

    size_t size = context.sizeWritten;

    // Check to reallocate opdata
    if (size > bytecode->capacity) {
        free(bytecode->opdata);
        bytecode->opdata = new char[size];
        bytecode->capacity = size;
    }

    bytecode->size = size;
    ca_assert(bytecode->size <= bytecode->capacity);

    // Second pass: Write data for real
    context.writePos = bytecode->opdata;
    context.sizeWritten = 0;
    context.nextStackIndex = 0;

    write_bytecode_for_top_level_branch(&context, branch);
}

void update_bytecode(Branch& branch)
{
    return update_bytecode(branch, &branch._bytecode);
}

void print_bytecode(std::ostream& out, BytecodeData* data)
{
    for (Iterator it(data); !it.finished(); ++it) {
        out << ((char*) *it - data->opdata) << ": ";
        print_operation(out, *it);
        out << std::endl;
    }
}

void print_bytecode(std::ostream& out, Branch& branch)
{
    return print_bytecode(out, &branch._bytecode);
}

void print_operation(std::ostream& out, Operation* op)
{
    switch (op->opid) {
        case OP_STACK_SIZE:
            out << "stack_size[" << ((StackSizeOperation*) op)->numElements << "]";
            break;
        case OP_CALL: {
            CallOperation *callOp = (CallOperation*) op;
            out << "call " << callOp->function->name;
            out << "(";
            for (int i=0; i < callOp->numInputs; i++) {
                if (i != 0) out << " ";
                out << callOp->inputs[i].stackIndex;
            }
            out << ")";
            if (callOp->outputIndex != -1)
                out << " -> " << callOp->outputIndex;
            break;
        }
        case OP_PUSH_VALUE: {
            PushValueOperation *pushOp = (PushValueOperation*) op;
            out << "push val:" << pushOp->source->toString();
            out << " -> " << pushOp->outputIndex;
            break;
        }
        case OP_JUMP: {
            JumpOperation *jumpOp = (JumpOperation*) op;
            out << "jump offset:" << jumpOp->offset;
            break;
        }
        case OP_JUMP_IF: {
            JumpIfOperation *jumpOp = (JumpIfOperation*) op;
            out << "jump_if(" << jumpOp->conditionIndex << ") offset:" << jumpOp->offset;
            break;
        }
        case OP_JUMP_IF_NOT: {
            JumpIfNotOperation *jumpOp = (JumpIfNotOperation*) op;
            out << "jump_if_not(" << jumpOp->conditionIndex << ") offset:" << jumpOp->offset;
            break;
        }
        case OP_RETURN:
            break;
        case OP_PUSH_INT: {
            PushIntOperation *pushOp = (PushIntOperation*) op;
            out << "push_int val:" << pushOp->value << " -> " << pushOp->outputIndex;
            break;
        }
        case OP_INCREMENT: {
            IncrementOperation *incOp = (IncrementOperation*) op;
            out << "increment " << incOp->stackIndex;
            break;
        }
        case OP_GET_INDEX: {
            GetIndexOperation *getOp = (GetIndexOperation*) op;
            out << "get_index(" << getOp->listIndex << "," << getOp->indexInList
                << ") -> " << getOp->outputIndex;
            break;
        }
        case OP_APPEND: {
            AppendOperation *appendOp = (AppendOperation*) op;
            out << "append(" << appendOp->itemIndex << ") -> " << appendOp->outputIndex;
            break;
        }
        case OP_NUM_ELEMENTS: {
            NumElementsOperation *neop = (NumElementsOperation*) op;
            out << "num_elements(" << neop->listIndex << ") -> " << neop->outputIndex;
            break;
        }
        case OP_COPY: {
            CopyOperation *copyop = (CopyOperation*) op;
            out << "copy(" << copyop->fromIndex << ") -> " << copyop->toIndex;
            break;
        }
        default: ca_assert(false);
    }
}

void print_bytecode_raw(std::ostream& out, BytecodeData* data)
{
    for (Iterator it(data); !it.finished(); ++it) {
        char* pos = (char*) *it;
        char* op_end = pos + get_operation_size(*it);
        char buf[10];
        for (; pos != op_end; pos++) {
            sprintf(buf, "%02X", *pos);
            out << buf;
        }
        out << std::endl;
    }
}

bool Iterator::finished()
{
    return pos == NULL;
}
Operation* Iterator::current()
{
    return pos;
}
void Iterator::advance()
{
    pos = (Operation*) (((char*) pos) + get_operation_size(pos));
    if (pos == end)
        pos = NULL;
    ca_assert(pos < end);
}

} // namespace bytecode
} // namespace circa
