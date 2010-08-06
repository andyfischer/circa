
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
    OP_RETURN
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
    Input inputs[0];
};

struct ReturnOperation {
    OpId opid;
    Term* caller;
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

void update_bytecode(Branch& branch, BytecodeData* bytecode);
void update_bytecode(Branch& branch);

void print_bytecode(std::ostream& out, BytecodeData* data);
void print_bytecode(std::ostream& out, Branch& branch);
void print_operation(std::ostream& out, Operation* op);

struct Iterator {
    BytecodeData* data;
    Operation* pos;

    Iterator(BytecodeData* data) : data(data), pos((Operation*) data->opdata) {}

    bool finished();
    Operation* current();
    void advance();
    Operation* operator*() { return current(); }
    void operator++() { advance(); }
};

} // namespace bytecode
} // namespace circa
