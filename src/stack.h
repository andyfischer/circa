// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "rand.h"

namespace circa {

struct FrameList
{
    Frame* frame;
    int count;
    int capacity;
};

struct HacksetBytecodeCache
{
    // Bytecode generated for one specific hackset.

    struct BlockEntry {
        Block* block;
        Value bytecode;
    };

    BlockEntry* blocks;
    int blockCount;

    Value indexMap; // Map of Block* to block index.
};

struct BytecodeCache
{
    HacksetBytecodeCache** byHackset;
    int count;

    Value hacksetMap; // Map of hackset value to index.
};

struct Stack
{
    // Globally unique ID.
    int id;

    // Activation frame list.
    FrameList frames;

    // Cached bytecode data. Can be regenerated.
    BytecodeCache bytecode;

    // Stored frame per module, keyed by block ID.
    Value moduleFrames;

    // Top-level context.
    Value topContext;

    // Stack-local random number generator.
    RandState randState;

    // Transient data, used during vm_run.
    HacksetBytecodeCache* currentHacksetBytecode;
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

    // Value slot, may be used by the stack's owner.
    Value context;

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

    // Register values.
    Value registers;

    // Stack state: data saved between invocations.
    Value state;

    // Outgoing stack state. Data that will be committed upon this frame's complemtion.
    Value outgoingState;

    // Source block
    Block* block;

    // Map of Term to value, for scope-specific bindings. Used in closure call.
    Value bindings;

    // Map of term->value. Used for a closure call.
    Value dynamicScope;

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
};

void stack_resize_frame_list(Stack* stack, int newCapacity);
Frame* stack_push_blank_frame(Stack* stack);
Stack* stack_duplicate(Stack* stack);

caValue* stack_active_value_for_block_id(Frame* frame, int blockId, int termIndex);
caValue* stack_active_value_for_term(Frame* frame, Term* term);

// Stack block cache
char* stack_bytecode_get_data(Stack* stack, int index);
Block* stack_bytecode_get_block(Stack* stack, int index);
int stack_bytecode_get_index_for_block(Stack* stack, Block* block);
int stack_bytecode_get_index(Stack* stack, caValue* key);
int stack_bytecode_create_entry(Stack* stack, Value* key);
int stack_bytecode_create_entry_for_block(Stack* stack, Block* block);

// Prepare stack's bytecode for a VM run. Load/create a cache entry relevant to the
// current hackset.
void stack_bytecode_start_run(Stack* stack);

void stack_bytecode_erase(Stack* stack);

void stack_get_hackset(Stack* stack, Value* hackset);

// Module frames
caValue* stack_module_frame_get(Stack* stack, int blockId);
caValue* stack_module_frame_save(Stack* stack, Block* block, caValue* registers);
Block* module_frame_get_block(caValue* moduleFrame);
caValue* module_frame_get_registers(caValue* moduleFrame);

void stack_on_migration(Stack* stack);

Frame* as_frame_ref(caValue* value);
bool is_frame_ref(caValue* value);
void set_frame_ref(caValue* value, Frame* frame);

void set_retained_frame(caValue* frame);
bool is_retained_frame(caValue* frame);
caValue* retained_frame_get_block(caValue* frame);
caValue* retained_frame_get_state(caValue* frame);

void copy_stack_frame_outgoing_state_to_retained(Frame* source, caValue* retainedFrame);

void frame_copy(Frame* left, Frame* right);

} // namespace circa
