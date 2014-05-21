// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "rand.h"

namespace circa {

struct Program;

struct Stack
{
    // Globally unique ID.
    int id;

    int refCount;
    bool isRefcounted;

    // Activation frame list. Has a list of Frame objects in sequential memory. Each
    // Frame object has variable size (see frame_size()), so this list can't be indexed.
    char* frameData;
    int framesCount;
    size_t framesCapacity;

    Frame* top;

    Value derivedHackset;
    Program* program;

    // Stored values that were computed on-demand. Keyed by Term reference.
    Value demandValues;

    // Top-level env.
    Value env;

    // Term value observations. Keyed by stack path.
    Value observations;

    // Stack-local random number generator.
    RandState randState;

    // Transient data, used during vm_run.
    char* bc;
    int pc;

    // Current step, either StackReady, StackRunning or StackFinished.
    Symbol step;

    // Flag that indicates the most recent run was interrupted by an error
    bool errorOccurred;

    // Linked list of all stacks across this world.
    Stack* prevStack;
    Stack* nextStack;

    // Owning world
    caWorld* world;

    // Miscellaneous attributes, used by owner.
    Value attrs;

    Stack();
    ~Stack();

    void dump();

private:
    // Disabled C++ functions.
    Stack(Stack const&) {}
    Stack& operator=(Stack const&) { return *this; }
};
    
struct Frame
{
    // Pointer to owning Stack.
    Stack* stack;

    // PC (in the parent frame) that this frame was expanded from. Invalid for the first frame.
    int parentIndex;

    // Size of previous frame, used when walking backwards.
    u32 prevFrameSize;

    // Stack state: data saved between invocations.
    Value state;

    // Outgoing stack state. Will be committed to parent frame's state.
    Value outgoingState;

    // Source block
    Block* block;

    // Map of Term to value, for scope-specific bindings. Used in closure call.
    Value bindings;

    // Map of term->value. Used for a closure call.
    Value dynamicScope;

    // Block index (as stored in BytecodeCache).
    int blockIndex;

    // Bytecode data. Data is owned in Stack.bytecode.
    char* bc;

    // Bytecode position. Saved when this frame is not the top.
    int pc;

    // PC stored as a termIndex.
    int termIndex;

    // Whether this frame was pushed from a normal call, or Func.apply/Func.call.
    Symbol callType;

    // When a block is exited early, this stores the exit type.
    Symbol exitType;

    int registerCount;
    Value registers[0]; // Has length of 'registerCount'
};

// Allocate a new Stack object.
Stack* create_stack(World* world);
void free_stack(Stack* stack);
void stack_incref(Stack* stack);
void stack_decref(Stack* stack);

Frame* stack_push_blank_frame(Stack* stack, int registerCount);
void stack_pop_no_retain(Stack* stack);

// Resize the given frame to have a new register count. This will invalidate your 'frame'
// pointer, so the return value is a valid pointer to the same frame.
Frame* stack_resize_frame(Stack* stack, Frame* frame, int newRegisterCount);

int stack_frame_count(Stack* stack);

Frame* stack_top_parent(Stack* stack);
Block* stack_top_block(Stack* stack);

// Returns whether evaluation has been stopped due to an error.
bool stack_errored(Stack* stack);

caValue* stack_env_insert(Stack* stack, caValue* name);
caValue* stack_env_get(Stack* stack, caValue* name);

void stack_extract_current_path(Stack* stack, caValue* path, Frame* untilFrame);

Stack* stack_duplicate(Stack* stack);

caValue* stack_active_value_for_block_index(Frame* frame, int blockIndex, int termIndex);
caValue* stack_active_value_for_term(Frame* frame, Term* term);

// Returns "first" frame; the first one to be executed; the first one in memory.
Frame* first_frame(Stack* stack);

// Returns "top" frame; the one that is currently executing; the last one in memory.
Frame* top_frame(Stack* stack);

Frame* next_frame(Frame* frame);
Frame* next_frame_n(Frame* frame, int distance);
Frame* prev_frame(Frame* frame);
Frame* prev_frame_n(Frame* frame, int distance);
size_t frame_size(Frame* frame);

caValue* frame_register(Frame* frame, int index);
caValue* frame_register(Frame* frame, Term* term);
caValue* frame_register_from_end(Frame* frame, int index);
int frame_register_count(Frame* frame);
void frame_registers_to_list(Frame* frame, caValue* list);
int frame_find_index(Frame* frame);
Term* frame_caller(Frame* frame);
Term* frame_term(Frame* frame, int index);
Term* frame_current_term(Frame* frame);
caValue* frame_state(Frame* frame);
Block* frame_block(Frame* frame);

// Prepare stack's bytecode for a VM run.
void stack_bytecode_start_run(Stack* stack);

void stack_on_program_change(Stack* stack);

void stack_derive_hackset(Stack* stack, Value* hackset);

caValue* stack_demand_value_insert(Stack* stack, Term* key);
caValue* stack_demand_value_get(Stack* stack, Term* key);

void stack_on_migration(Stack* stack);

Frame* as_frame_ref(caValue* value);
bool is_frame_ref(caValue* value);
void set_frame_ref(caValue* value, Frame* frame);

void set_retained_frame(caValue* frame);
bool is_retained_frame(caValue* frame);
caValue* retained_frame_get_block(caValue* frame);
caValue* retained_frame_get_state(caValue* frame);

void copy_stack_frame_outgoing_state_to_retained(Frame* source, caValue* retainedFrame);

void stack_setup_type(Type* stackType);

} // namespace circa
