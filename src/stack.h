// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "rand.h"

namespace circa {

struct Stack
{
    // Globally unique ID.
    int id;

    int refCount;
    bool isRefcounted;

    Frame* frames;
    int frameCount;
    int frameCapacity;

    Value* registers;
    int registerCount;
    int registerCapacity;

    Value derivedHackset;
    Compiled* program;

    // Stored values that were computed on-demand. Keyed by Term reference.
    Value demandValues;

    // Top-level env.
    Value env;

    Value observations;

    // Stack state
    Value state;

    // Stack-local random number generator.
    RandState randState;

    // Program counter, only valid during vm_run.
    char* bytecode;
    int pc;

    // Current step, either StackReady, StackRunning or StackFinished.
    Symbol step;

    // Flag that indicates the most recent run was interrupted by an error
    bool errorOccurred;

    // Linked list of all stacks across this world.
    Stack* prevStack;
    Stack* nextStack;

    // When this stack is running, 'caller' is the Stack that triggered the call.
    Stack* caller;

    // Owning world
    caWorld* world;

    // Miscellaneous attributes, used by owner.
    Value attrs;

    Stack();
    ~Stack();

    void dump();
    void dump_compiled();

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

    // Map of Term to value, for scope-specific bindings. Used in closure call.
    Value bindings;

    // Frame env values.
    Value env;

    Value incomingState;
    Value outgoingState;

    Block* block;

    // Block index (as stored in Program).
    int blockIndex;

    // Bytecode position. Saved when this frame is not the top.
    int pc;

    // PC stored as a termIndex.
    int termIndex;

    int firstRegisterIndex;
    int registerCount;
};

// Allocate a new Stack object.
Stack* create_stack(World* world);
void free_stack(Stack* stack);
void stack_incref(Stack* stack);
void stack_decref(Stack* stack);

Frame* stack_push_blank_frame(Stack* stack, int registerCount);
void stack_resize_top_frame(Stack* stack, int registerCount);
void pop_frame(Stack* stack);

// Resize the given frame to have a new register count. This will invalidate your 'frame'
// pointer, so the return value is a valid pointer to the same frame.
Frame* stack_resize_frame(Stack* stack, Frame* frame, int newRegisterCount);

int stack_frame_count(Stack* stack);

Frame* top_frame_parent(Stack* stack);
Block* stack_top_block(Stack* stack);

caValue* stack_state(Stack* stack);

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

CompiledBlock* frame_compiled_block(Frame* frame);

Frame* next_frame(Frame* frame);
Frame* next_frame_n(Frame* frame, int distance);
Frame* prev_frame(Frame* frame);
Frame* prev_frame_n(Frame* frame, int distance);
size_t frame_size(Frame* frame);

caValue* stack_register(Stack* stack, int index);
caValue* stack_register_rel(Stack* stack, int relativeIndex);
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
void stack_save_watch_observation(Stack* stack, Value* path, Value* value);
Value* stack_get_watch_observation(Stack* stack, Value* path);

void stack_on_migration(Stack* stack);

Frame* as_frame_ref(caValue* value);
bool is_frame_ref(caValue* value);
void set_frame_ref(caValue* value, Frame* frame);

void stack_setup_type(Type* stackType);
void stack_install_functions(NativePatch* patch);

} // namespace circa
