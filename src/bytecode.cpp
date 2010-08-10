// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch_iterator.h"
#include "builtins.h"
#include "bytecode.h"
#include "debug.h"
#include "if_block.h"
#include "introspection.h"
#include "for_loop.h"
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

    if (context && should_term_output_go_on_stack(term)
            && term->stackIndex == -1)
        term->stackIndex = context->nextStackIndex++;

    const int MAX_INPUTS_EVER = 1000;
    int inputIndexes[MAX_INPUTS_EVER];

    for (int i=0; i < numInputs; i++) {
        Term* input = term->input(i);
        ca_assert(input->stackIndex != -1);
        inputIndexes[i] = input->stackIndex;
    }

    write_call_op(context, term, term->function, numInputs, inputIndexes, term->stackIndex);
}

void write_push_value_op(WriteContext* context, Term* term)
{
    if (context && term->stackIndex == -1) {
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

void write_jump_op(WriteContext* context, size_t offset)
{
    if (context->writePos) {
        JumpOperation* op = (JumpOperation*) context->writePos;
        op->opid = OP_JUMP;
        op->offset = offset;
    }
    context->advance(sizeof(JumpOperation));
}

void write_jump_if_op(WriteContext* context, int conditionIndex, size_t offset)
{
    if (context->writePos) {
        JumpIfOperation* op = (JumpIfOperation*) context->writePos;
        op->opid = OP_JUMP_IF;
        op->conditionIndex = conditionIndex;
        op->offset = offset;
    }
    context->advance(sizeof(JumpIfOperation));
}
void write_jump_if_not_op(WriteContext* context, int conditionIndex, size_t offset)
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
    if (term->function == RETURN_FUNC)
        return write_return_op(context, term);

    if (term->function == VALUE_FUNC)
        return write_push_value_op(context, term);

    if (term->function == IF_BLOCK_FUNC)
        return write_if_block_bytecode(context, term);

    if (term->function == FOR_FUNC)
        return write_for_loop_bytecode(context, term);

    return write_call_op(context, term);
}

void update_bytecode(Branch& branch, BytecodeData* bytecode)
{
    // Clear stack indexes for all terms
    for (BranchIterator it(branch); !it.finished(); ++it)
        it->stackIndex = -1;

    // First pass: Figure out the size of generated bytecode, update stack
    // indexes of terms, and count the # of elements on the stack.
    WriteContext context;
    context.nextStackIndex = 0;
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];

        if (!should_term_generate_call(term))
            continue;

        write_op(&context, term);
    }

    // Add space for a STACK_SIZE op
    context.advance(sizeof(StackSizeOperation));

    size_t size = context.sizeWritten;

    // Check to reallocate opdata
    if (size > bytecode->capacity) {
        free(bytecode->opdata);
        bytecode->opdata = new char[size];
        bytecode->capacity = size;
    }

    bytecode->size = size;
    ca_assert(bytecode->size <= bytecode->capacity);

    // Start writing operations
    context.writePos = bytecode->opdata;
    context.sizeWritten = 0;
    
    // First add a STACK_SIZE op
    int stackElements = context.nextStackIndex;
    write_stack_size_op(&context, stackElements);

    // Write remaining ops
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (!should_term_generate_call(term))
            continue;

        write_op(&context, term);
        ca_assert(size_t(context.writePos - bytecode->opdata) <= bytecode->size);
    }
}

void update_bytecode(Branch& branch)
{
    return update_bytecode(branch, &branch._bytecode);
}

void print_bytecode(std::ostream& out, BytecodeData* data)
{
    for (Iterator it(data); !it.finished(); ++it) {
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
            out << "stack_size " << ((StackSizeOperation*) op)->numElements;
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
            out << "push " << pushOp->source->toString();
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
            out << "push_int " << pushOp->value << " -> " << pushOp->outputIndex;
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
