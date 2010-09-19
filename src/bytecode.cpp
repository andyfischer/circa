// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "branch_iterator.h"
#include "builtins.h"
#include "bytecode.h"
#include "debug.h"
#include "evaluation.h"
#include "function.h"
#include "if_block.h"
#include "introspection.h"
#include "for_loop.h"
#include "subroutine.h"
#include "stateful_code.h"
#include "term.h"
#include "type.h"

#define VERBOSE_PRINT_VM 0

namespace circa {
namespace bytecode {

WriteContext::WriteContext(BytecodeData* _bytecode)
  : bytecode(_bytecode),
    nextStackIndex(0),
    topLevelState(-1),
    inlineState(-1)
{}

void
WriteContext::guaranteeSize(size_t moreBytes)
{
    size_t neededSize = bytecode->size + moreBytes;
    if (neededSize > bytecode->capacity) {
        resize_opdata(bytecode, std::max(neededSize, bytecode->capacity*2));
    }
}

void
WriteContext::advance(size_t bytes) {
    ca_assert((bytecode->size + bytes) <= bytecode->capacity);
    bytecode->size += bytes;
}
int
WriteContext::getOffset()
{
    return int(bytecode->size);
}
Operation*
WriteContext::writePos()
{
    ca_assert(bytecode->size < bytecode->capacity);
    return (Operation*) (bytecode->opdata + bytecode->size);
}
BytecodePosition
WriteContext::getPosition()
{
    return BytecodePosition(bytecode, bytecode->size);
}
int
WriteContext::appendLocal(TaggedValue* val)
{
    int index = bytecode->locals.length();
    bytecode->locals.append(val);
    return index;
}

void
WriteContext::appendStateFieldStore(std::string const& fieldName, int nameRegister)
{
    PendingStateFieldStore store;
    store.fieldName = fieldName;
    store.nameRegister = nameRegister;
    pendingStateFieldStores.push_back(store);
}

size_t get_operation_size(Operation* op)
{
    switch (op->opid) {
        case OP_STACK_SIZE: return sizeof(StackSizeOperation);
        case OP_CALL: {
            CallOperation *callOp = (CallOperation*) op;
            return sizeof(CallOperation) + sizeof(CallOperation::Input)*callOp->numInputs;
        }
        case OP_PUSH_VALUE: return sizeof(PushValueOperation);
        case OP_PUSH_LOCAL: return sizeof(PushLocalOperation);
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
        case OP_RAISE: return sizeof(RaiseOperation);
        case OP_CHECK_ERROR: return sizeof(CheckErrorOperation);
        case OP_COMMENT: {
            CommentOperation *commentop = (CommentOperation*) op;
            return commentop->size;
        }
        case OP_VAR_NAME: {
            VarNameOperation *nameop = (VarNameOperation*) op;
            return nameop->size;
        }
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
    if (term == NULL)
        return false;
    if (term->function == COMMENT_FUNC)
        return false;
    if (term->boolPropOptional("no-bytecode", false))
        return false;
    if (is_function(term))
        return false;
    if (is_type(term))
        return false;
    if (is_function_attrs(term))
        return false;

    return true;
}

void write_stack_size_op(WriteContext* context, int stacksize)
{
    size_t size = sizeof(StackSizeOperation);
    context->guaranteeSize(size);
    StackSizeOperation* op = (StackSizeOperation*) context->writePos();
    op->opid = OP_STACK_SIZE;
    op->numElements = stacksize;
    context->advance(size);
}

void write_check_error(WriteContext* context)
{
    size_t size = sizeof(CheckErrorOperation);
    context->guaranteeSize(size);
    CheckErrorOperation* op = (CheckErrorOperation*) context->writePos();
    op->opid = OP_CHECK_ERROR;
    context->advance(size);
}

void write_call_op(WriteContext* context, Term* caller, Term* function, int numInputs, int* inputIndexes, int outputIndex)
{
    size_t size = sizeof(CallOperation) + sizeof(CallOperation::Input)*numInputs;
    context->guaranteeSize(size);

    CallOperation* op = (CallOperation*) context->writePos();
    op->opid = OP_CALL;
    op->caller = caller;
    op->function = function;
    op->numInputs = numInputs;
    op->outputIndex = outputIndex;

    for (int i=0; i < numInputs; i++)
        op->inputs[i].stackIndex = inputIndexes[i];

    context->advance(size);

    // If this call can throw error, add a check_error op
    if (function_t::get_attrs(function).throws)
        write_check_error(context);
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
    size_t size = sizeof(PushValueOperation);
    context->guaranteeSize(size);

    if (term->stackIndex == -1) {
        ca_assert(term->function->name != "trace");
        term->stackIndex = context->nextStackIndex++;
    }

    PushValueOperation* op = (PushValueOperation*) context->writePos();
    op->opid = OP_PUSH_VALUE;
    op->outputIndex = term->stackIndex;
    op->source = term;
    
    context->advance(size);
}

int write_push_local_op(WriteContext* context, TaggedValue* value)
{
    size_t size = sizeof(PushLocalOperation);
    context->guaranteeSize(size);

    PushLocalOperation* op = (PushLocalOperation*) context->writePos();
    op->opid = OP_PUSH_LOCAL;
    op->localIndex = context->bytecode->locals.length();
    op->output = context->nextStackIndex++;
    context->bytecode->locals.append(value);
    context->advance(size);
    return op->output;
}

void write_jump(WriteContext* context, int offset)
{
    size_t size = sizeof(JumpOperation);
    context->guaranteeSize(size);
    JumpOperation* op = (JumpOperation*) context->writePos();
    op->opid = OP_JUMP;
    op->offset = offset;
    context->advance(size);
}

void write_jump_if(WriteContext* context, int conditionIndex, int offset)
{
    size_t size = sizeof(JumpIfOperation);
    context->guaranteeSize(size);
    JumpIfOperation* op = (JumpIfOperation*) context->writePos();
    op->opid = OP_JUMP_IF;
    op->conditionIndex = conditionIndex;
    op->offset = offset;
    context->advance(size);
}
void write_jump_if_not(WriteContext* context, int conditionIndex, int offset)
{
    size_t size = sizeof(JumpIfNotOperation);
    context->guaranteeSize(size);
    JumpIfNotOperation* op = (JumpIfNotOperation*) context->writePos();
    op->opid = OP_JUMP_IF_NOT;
    op->conditionIndex = conditionIndex;
    op->offset = offset;
    context->advance(size);
}
void write_push_int(WriteContext* context, int value, int outputIndex)
{
    size_t size = sizeof(PushIntOperation);
    context->guaranteeSize(size);
    PushIntOperation* op = (PushIntOperation*) context->writePos();
    op->opid = OP_PUSH_INT;
    op->value = value;
    op->outputIndex = outputIndex;
    context->advance(size);
}
void write_get_index(WriteContext* context, int listIndex, int indexInList, int outputIndex)
{
    size_t size = sizeof(GetIndexOperation);
    context->guaranteeSize(size);
    GetIndexOperation* op = (GetIndexOperation*) context->writePos();
    op->opid = OP_GET_INDEX;
    op->listIndex = listIndex;
    op->indexInList = indexInList;
    op->outputIndex = outputIndex;
    context->advance(size);
}
void write_increment(WriteContext* context, int intIndex)
{
    size_t size = sizeof(IncrementOperation);
    context->guaranteeSize(size);
    IncrementOperation* op = (IncrementOperation*) context->writePos();
    op->opid = OP_INCREMENT;
    op->stackIndex = intIndex;
    context->advance(size);
}
void write_num_elements(WriteContext* context, int listIndex, int outputIndex)
{
    size_t size = sizeof(NumElementsOperation);
    context->guaranteeSize(size);
    NumElementsOperation* op = (NumElementsOperation*) context->writePos();
    op->opid = OP_NUM_ELEMENTS;
    op->listIndex = listIndex;
    op->outputIndex = outputIndex;
    context->advance(size);
}

void write_copy(WriteContext* context, int fromIndex, int toIndex)
{
    size_t size = sizeof(CopyOperation);
    context->guaranteeSize(size);
    CopyOperation* op = (CopyOperation*) context->writePos();
    op->opid = OP_COPY;
    op->fromIndex = fromIndex;
    op->toIndex = toIndex;
    context->advance(size);
}
void write_raise(WriteContext* context)
{
    size_t size = sizeof(RaiseOperation);
    context->guaranteeSize(size);
    RaiseOperation* op = (RaiseOperation*) context->writePos();
    op->opid = OP_RAISE;
    context->advance(size);
}

void write_comment(WriteContext* context, const char* str)
{
    // TODO: Compiler or runtime switch that can turn off comments

    size_t size = sizeof(CommentOperation) + strlen(str) + 1;
    // Round up size to multiple of 4 bytes
    size = (size + 3) & ~0x3;

    context->guaranteeSize(size);
    CommentOperation* op = (CommentOperation*) context->writePos();
    op->opid = OP_COMMENT;
    op->size = size;
    strcpy(op->text, str);
    context->advance(size);
}
void write_var_name(WriteContext* context, int stackIndex, const char* name)
{
    // todo
}

void write_return_op(WriteContext* context, Term* term)
{
    size_t size = sizeof(ReturnOperation);
    context->guaranteeSize(size);
    ReturnOperation* op = (ReturnOperation*) context->writePos();
    op->opid = OP_RETURN;
    op->caller = term;
    op->stackIndex = -1;
    if (term->input(0) != NULL)
        op->stackIndex = term->input(0)->stackIndex;
    context->advance(size);
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

void assign_stack_index(WriteContext* context, Term* term)
{
    if (term->stackIndex == -1)
        term->stackIndex = context->nextStackIndex++;
}

void assign_stack_for_major_branch(WriteContext* context, Branch& branch)
{
    // Clean up existing stack indices
    for (BranchIterator it(branch); !it.finished(); ++it) {
        // Don't touch major branches
        if (is_subroutine(*it)) {
            it.skipNextBranch();
        }
        it->stackIndex = -1;
    }

    // Scan for input() terms, these must get certain stack indexes.
    ca_assert(context->nextStackIndex == 0);
    for (int i=0; i < branch.length(); i++) {
        if (branch[i]->function == INPUT_PLACEHOLDER_FUNC)
            branch[i]->stackIndex = context->nextStackIndex++;
    }
}

int write_bytecode_for_branch(WriteContext* context, Branch& branch, int inlineState)
{
    int prevInlineState = context->inlineState;

    context->inlineState = inlineState;

    int lastStackIndex = -1;

    for (int i=0; i < branch.length(); i++)
        write_op(context, branch[i]);

    // Find stack index of last expression
    for (int i=branch.length()-1; i >= 0; i--) {
        if (should_term_generate_call(branch[i])) {
            lastStackIndex = branch[i]->stackIndex;
            break;
        }
    }

    // Wrap up any state vars that were declared in this branch.
    for (size_t i=0; i < context->pendingStateFieldStores.size(); i++) {
        WriteContext::PendingStateFieldStore &store = context->pendingStateFieldStores[i];

        Term* result = branch[store.fieldName];

        int inputs[] = { context->inlineState, store.nameRegister, result->stackIndex };
        write_call_op(context, NULL, get_global("set_state_field"), 3, inputs,
                context->inlineState);
    }
    context->pendingStateFieldStores.clear();

    context->inlineState = prevInlineState;
    return lastStackIndex;
}

void write_bytecode_for_branch_inline(WriteContext* context, Branch& branch)
{
    for (int i=0; i < branch.length(); i++)
        write_op(context, branch[i]);
}
void write_raise_if(WriteContext* context, Term* errorCondition)
{
    BytecodePosition jumpIfNot = context->getPosition();
    write_jump_if_not(context, errorCondition->stackIndex, 0);
    write_raise(context);
    ((JumpIfNotOperation*) jumpIfNot.get())->offset = context->getOffset();
}

void write_get_state_field(WriteContext* context, Term* term, int name,
        int defaultValue, int output)
{
    ca_assert(context->topLevelState != -1);
    ca_assert(name != -1);
    int inputs[] = { context->inlineState, name, defaultValue };
    bytecode::write_call_op(context, term, get_global("get_state_field"), 3, inputs,
            output);
}

void write_bytecode_for_top_level_branch(WriteContext* context, Branch& branch)
{
    assign_stack_for_major_branch(context, branch);

    // First add a STACK_SIZE op. Will fill in the actual size later.
    BytecodePosition stackSize = context->getPosition();
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
       
    ((StackSizeOperation*) stackSize.get())->numElements = context->nextStackIndex;
}

void resize_opdata(BytecodeData* bytecode, size_t newCapacity)
{
    bytecode->capacity = newCapacity;
    bytecode->opdata = (char*) realloc(bytecode->opdata, newCapacity);
}

void update_bytecode(Branch& branch, BytecodeData* bytecode)
{
    ca_assert(!bytecode->inuse);

    bytecode->size = 0;
    bytecode->locals.clear();

    const size_t default_size = 256;

    if (bytecode->capacity < default_size)
        resize_opdata(bytecode, default_size);

    WriteContext context(bytecode);
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
        print_operation(out, data, *it);
        out << std::endl;
    }
}

void print_bytecode(std::ostream& out, Branch& branch)
{
    update_bytecode(branch);
    return print_bytecode(out, &branch._bytecode);
}

void print_operation(std::ostream& out, BytecodeData* bytecode, Operation* op)
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
        case OP_PUSH_LOCAL: {
            PushLocalOperation *pushOp = (PushLocalOperation*) op;
            out << "push local(" << bytecode->locals[pushOp->localIndex]->toString();
            out << ") -> " << pushOp->output;
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
        case OP_RETURN: {
            ReturnOperation *retop = (ReturnOperation*) op;
            out << "return(" << retop->stackIndex << ")";
            break;
        }
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
        case OP_RAISE: {
            RaiseOperation *raiseop = (RaiseOperation*) op;
            out << "raise(" << raiseop->value << ")";
            break;
        }
        case OP_CHECK_ERROR: {
            out << "check_error";
            break;
        }
        case OP_COMMENT: {
            CommentOperation *commentop = (CommentOperation*) op;
            out << "# " << commentop->text;
            break;
        }
        case OP_VAR_NAME: {
            VarNameOperation *nameop = (VarNameOperation*) op;
            out << nameop->name << ": " << nameop->stackIndex;
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

void print_bytecode_for_all_major_branches(std::ostream& out, Branch& branch)
{
    for (int i=0; i < branch.length(); i++) {
        Term* term = branch[i];
        if (is_subroutine(term)) {
            update_bytecode(term->nestedContents);
            out << "[" << term->name << "]" << std::endl;
            print_bytecode(out, term->nestedContents);
        }
    }

    out << "[#main]" << std::endl;
    update_bytecode(branch);
    print_bytecode(out, branch);
}

CallOperation* create_orphan_call_operation(Term* caller, Term* function, int numInputs)
{
    CallOperation* op = (CallOperation*) malloc(sizeof(CallOperation)
            + sizeof(CallOperation::Input) * numInputs);
    op->opid = OP_CALL;
    op->caller = caller;
    op->function = function;
    op->numInputs = numInputs;
    return op;
}
void free_orphan_call_operation(CallOperation* op)
{
    free(op);
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

void evaluate_bytecode(EvalContext* cxt, BytecodeData* data, List* stack)
{
    //std::cout << "running bytecode:" << std::endl;
    //print_bytecode(std::cout, data);

    //cxt->clearError();
    data->inuse = true;

    char* pos = data->opdata;
    char* end = data->opdata + data->size;

    while (pos != end) {

        bytecode::Operation* op = (Operation*) pos;

        #if VERBOSE_PRINT_VM
            std::cout << std::endl << "stack: " << stack->toString() << std::endl;
            std::cout << "state: " << cxt->topLevelState.toString() << std::endl;
            std::cout << "next op: ";
            print_operation(std::cout, data, op);
        #endif

        switch (op->opid) {
            case bytecode::OP_STACK_SIZE: {
                bytecode::StackSizeOperation *ssop = (bytecode::StackSizeOperation*) op;
                stack->resize(ssop->numElements);
                pos += sizeof(bytecode::StackSizeOperation);
                continue;
            }
            case bytecode::OP_PUSH_VALUE: {
                bytecode::PushValueOperation *pushOp = (bytecode::PushValueOperation*) op;
                TaggedValue* output = stack->get(pushOp->outputIndex);
                ca_assert(output != NULL);
                change_type(output, type_contents(pushOp->source->type));
                copy(pushOp->source, output);
                pos += sizeof(bytecode::PushValueOperation);
                continue;
            }
            case bytecode::OP_PUSH_LOCAL: {
                bytecode::PushLocalOperation *pushOp = (bytecode::PushLocalOperation*) op;
                TaggedValue* val = data->locals.get(pushOp->localIndex);
                TaggedValue* output = stack->get(pushOp->output);
                ca_assert(val != NULL);
                copy(val,output);
                pos += sizeof(bytecode::PushLocalOperation);
                continue;
            }
            case bytecode::OP_CALL: {
                bytecode::CallOperation *callop = (bytecode::CallOperation*) op;

                EvaluateFunc func = function_t::get_attrs(callop->function).evaluate;
                func(cxt, stack, callop);

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
                    TaggedValue* v = stack->get(retop->stackIndex);
                    copy(v, &cxt->subroutineOutput);
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
                make_int(value, as_int(value) + 1);
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

} // namespace bytecode
} // namespace circa
