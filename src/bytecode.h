
// Goals:
//
// Do as much pre-processing of code as possible. This includes:
//   Resolving overloaded function calls (if possible)
//   Figuring out the return value for function calls
//   Insert code to preserve state vars
//   Assign stack registers
//
// Outline:
//   Each bytecode entry will have:
//     Pointer to function's term
//     Pointer to original term
//     List of inputs. Each input has:
//       Type of input location: either Stack or Static
//       The location (for Stack, an index, for Static, a pointer)
//     Stack index for output
//
// Algorithm:
//
// For a block:
//   Iterate through each item, count the # of items that should go on the stack
//     (In this loop, also assign indexes to the Term)
//   Record the stack size in bytecode output
//   For each term:
//     Should we skip this term? (yes for comments/whitespace)
//     Write an entry with function, caller, inputs
//
// Bytecode ops:
//
// stack_size - First op in a branch, 2nd arg declares the # of items on the stack
// call - Variable sized, call a function with the given function,caller,inputs
// return - Return, has one input: the stack index of the return value


// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {
namespace bytecode {

enum OpId {
    OP_STACK_SIZE = 1,
    OP_CALL,
    OP_PUSH_VALUE,
    OP_JUMP,
    OP_JUMP_IF,
    OP_JUMP_IF_NOT,
    OP_RETURN,
    OP_PUSH_INT,
    OP_INCREMENT,
    OP_GET_INDEX,
    OP_APPEND,
    OP_NUM_ELEMENTS,
    OP_COPY
};

struct Operation {
    OpId opid;
    int data[0];
};

struct StackSizeOperation {
    OpId opid;
    int numElements; 
};

struct CallOperation {
    struct Input {
        int stackIndex;
    };

    OpId opid;
    Term* caller;
    Term* function;
    int numInputs;
    int outputIndex;
    Input inputs[0];
};

struct PushValueOperation {
    OpId opid;
    Term* source;
    int outputIndex;
};

struct ReturnOperation {
    OpId opid;
    Term* caller;
};

struct JumpOperation {
    OpId opid;
    int offset;
};

struct JumpIfOperation {
    OpId opid;
    int conditionIndex;
    int offset;
};

struct JumpIfNotOperation {
    OpId opid;
    int conditionIndex;
    int offset;
};

struct PushIntOperation {
    OpId opid;
    int value;
    int outputIndex;
};
struct IncrementOperation {
    OpId opid;
    int stackIndex;
};
struct GetIndexOperation {
    OpId opid;
    int listIndex;
    int indexInList;
    int outputIndex;
};
struct AppendOperation {
    OpId opid;
    int itemIndex;
    int outputIndex;
};
struct NumElementsOperation {
    OpId opid;
    int listIndex;
    int outputIndex;
};
struct CopyOperation {
    OpId opid;
    int fromIndex;
    int toIndex;
};

struct WriteContext {
    int nextStackIndex;
    size_t sizeWritten;
    char* writePos;

    // Stack index of top level state
    int topLevelState;

    // Stack index of state object for the current branch
    int inlineState;

    WriteContext()
      : nextStackIndex(0),
        sizeWritten(0),
        writePos(NULL),
        topLevelState(-1),
        inlineState(-1)
    {}

    void advance(size_t bytes) {
        if (writePos)
            writePos += bytes;
        sizeWritten += bytes;
    }

    int getOffset() { return int(sizeWritten); }
};

struct BytecodeData {
    bool needsUpdate;
    size_t size;
    size_t capacity;
    char* opdata;

    BytecodeData(): needsUpdate(true), size(0), capacity(0), opdata(NULL) {}
};

// The size of the operation, in words.
size_t get_operation_size(Operation* op);
bool should_term_output_go_on_stack(Term* term);
bool should_term_generate_call(Term* term);

void write_call_op(WriteContext* context, Term* caller, Term* function, int numInputs, int* inputIndexes, int outputIndex);
void write_call_op(WriteContext* context, Term* term);
void write_jump(WriteContext* context, int offset);
void write_jump_if(WriteContext* context, int conditionIndex, int offset);
void write_jump_if_not(WriteContext* context, int conditionIndex, int offset);
void write_push_int(WriteContext* context, int value, int stackIndex);
void write_get_index(WriteContext* context, int listIndex, int indexInList, int outputIndex);
void write_increment(WriteContext* context, int intIndex);
void write_num_elements(WriteContext* context, int listIndex, int outputIndex);
void write_copy(WriteContext* context, int fromIndex, int toIndex);
void write_op(WriteContext* context, Term* term);
void write_bytecode_for_branch(WriteContext* context, Branch& branch, int inlineState,
        int firstIndex=0, int lastIndex=-1);

void update_bytecode(Branch& branch, BytecodeData* bytecode);
void update_bytecode(Branch& branch);

void print_bytecode(std::ostream& out, BytecodeData* data);
void print_bytecode(std::ostream& out, Branch& branch);
void print_operation(std::ostream& out, Operation* op);
void print_bytecode_raw(std::ostream& out, BytecodeData* data);

struct Iterator {
    Operation* pos;
    Operation* end;

    Iterator(BytecodeData* data)
      : pos((Operation*) data->opdata),
        end((Operation*) (data->opdata + data->size)) {}

    bool finished();
    Operation* current();
    void advance();
    Operation* operator*() { return current(); }
    Operation* operator->() { return current(); }
    void operator++() { advance(); }
};

} // namespace bytecode
} // namespace circa
