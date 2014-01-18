// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "bytecode.h"
#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
#include "list.h"
#include "stack.h"
#include "tagged_value.h"
#include "type.h"
#include "world.h"

namespace circa {

Stack::Stack()
 : errorOccurred(false),
   world(NULL)
{
    id = global_world()->nextStackID++;

    refCount = 1;
    isRefcounted = false;

    step = sym_StackReady;
    frames.capacity = 0;
    frames.count = 0;
    frames.frame = NULL;
    nextStack = NULL;
    prevStack = NULL;
    
    set_hashtable(&env);
    set_hashtable(&demandValues);
    rand_init(&randState, 0);

    bytecode.blocks = NULL;
    bytecode.blockCount = 0;
    set_hashtable(&bytecode.indexMap);
}

Stack::~Stack()
{
    // Clear error, so that stack_pop doesn't complain about losing an errored frame.
    stack_ignore_error(this);

    stack_bytecode_erase(this);
    stack_reset(this);

    free(frames.frame);

    if (world != NULL) {
        if (world->firstStack == this)
            world->firstStack = world->firstStack->nextStack;
        if (world->lastStack == this)
            world->lastStack = world->lastStack->prevStack;
        if (nextStack != NULL)
            nextStack->prevStack = prevStack;
        if (prevStack != NULL)
            prevStack->nextStack = nextStack;
    }
}

void
Stack::dump()
{
    circa::dump(this);
}

Stack* create_stack(World* world)
{
    Stack* stack = new Stack();

    stack->world = world;

    stack->isRefcounted = true;
    stack->refCount = 1;
    
    // Add to list of stacks in World.
    if (world != NULL) {
        if (world->firstStack == NULL)
            world->firstStack = stack;
        if (world->lastStack != NULL)
            world->lastStack->nextStack = stack;
        stack->prevStack = world->lastStack;
        stack->nextStack = NULL;
        world->lastStack = stack;
    }

    return stack;
}

void free_stack(Stack* stack)
{
    delete stack;
}

void stack_incref(Stack* stack)
{
    if (stack->isRefcounted) {
        ca_assert(stack->refCount > 0);
        stack->refCount++;
    }
}

void stack_decref(Stack* stack)
{
    if (stack->isRefcounted) {
        ca_assert(stack->refCount > 0);
        stack->refCount--;
        if (stack->refCount == 0)
            free_stack(stack);
    }
}

void stack_resize_frame_list(Stack* stack, int newCapacity)
{
    // Currently, the frame list can only be grown.
    ca_assert(newCapacity >= stack->frames.capacity);

    int oldCapacity = stack->frames.capacity;
    stack->frames.capacity = newCapacity;
    stack->frames.frame = (Frame*) realloc(stack->frames.frame,
        sizeof(Frame) * stack->frames.capacity);

    for (int i = oldCapacity; i < newCapacity; i++) {
        // Initialize newly allocated frame.
        Frame* frame = &stack->frames.frame[i];
        frame->stack = stack;
        initialize_null(&frame->registers);
        initialize_null(&frame->bindings);
        initialize_null(&frame->dynamicScope);
        initialize_null(&frame->state);
        initialize_null(&frame->outgoingState);
        frame->block = 0;
    }
}

Frame* stack_push_blank_frame(Stack* stack)
{
    // Check capacity.
    if ((stack->frames.count + 1) >= stack->frames.capacity)
        stack_resize_frame_list(stack, stack->frames.capacity == 0 ? 8 : stack->frames.capacity * 2);

    stack->frames.count++;

    Frame* frame = stack_top(stack);

    // Prepare frame.
    frame->termIndex = 0;
    frame->bc = NULL;
    frame->blockIndex = -1;
    frame->pc = 0;
    frame->exitType = sym_None;
    frame->callType = sym_NormalCall;
    frame->block = NULL;

    return frame;
}

Stack* stack_duplicate(Stack* stack)
{
    Stack* dupe = create_stack(stack->world);
    stack_resize_frame_list(dupe, stack->frames.capacity);

    for (int i=0; i < stack->frames.capacity; i++) {
        Frame* sourceFrame = &stack->frames.frame[i];
        Frame* dupeFrame = &dupe->frames.frame[i];

        frame_copy(sourceFrame, dupeFrame);
    }

    dupe->frames.count = stack->frames.count;
    dupe->step = stack->step;
    dupe->errorOccurred = stack->errorOccurred;
    set_value(&dupe->context, &stack->context);
    set_value(&dupe->env, &stack->env);
    return dupe;
}

caValue* stack_active_value_for_block_index(Frame* frame, int blockIndex, int termIndex)
{
    Stack* stack = frame->stack;

    while (true) {
        if (frame->blockIndex == blockIndex) {
            Term* term = frame_term(frame, termIndex);
            if (is_value(term))
                return term_value(term);
            return frame_register(frame, term);
        }

        frame = frame_parent(frame);

        if (frame == NULL)
            break;
    }

    // Check demandValues.
    Block* block = stack_bytecode_get_block(stack, blockIndex);
    Term* term = block->get(termIndex);

    caValue* demandValue = stack_demand_value_get(stack, term);
    if (demandValue != NULL)
        return demandValue;

    return NULL;
}

caValue* stack_active_value_for_term(Frame* frame, Term* term)
{
    int blockIndex = stack_bytecode_get_index(frame->stack, term->owningBlock);
    if (blockIndex == -1)
        return NULL;
    return stack_active_value_for_block_index(frame, blockIndex, term->index);
}

char* stack_bytecode_get_data(Stack* stack, int index)
{
    BytecodeCache* cache = &stack->bytecode;

    ca_assert(index < cache->blockCount);
    return as_blob(&cache->blocks[index].bytecode);
}

Block* stack_bytecode_get_block(Stack* stack, int index)
{
    BytecodeCache* cache = &stack->bytecode;

    ca_assert(index < cache->blockCount);
    return cache->blocks[index].block;
}

int stack_bytecode_get_index(Stack* stack, Block* block)
{
    BytecodeCache* cache = &stack->bytecode;

    Value key;
    set_block(&key, block);
    caValue* indexVal = hashtable_get(&cache->indexMap, &key);
    if (indexVal == NULL)
        return -1;
    return as_int(indexVal);
}

void stack_bytecode_generate_bytecode(Stack* stack, int blockIndex)
{
    BytecodeCache* cache = &stack->bytecode;
    BytecodeCache::BlockEntry* entry = &cache->blocks[blockIndex];

    if (!is_null(&entry->bytecode))
        return;

    Value bytecode;
    Block* block = NULL;
    bytecode_write_block(stack, &bytecode, entry->block);

    // Save bytecode. Re-lookup the entry because the above bytecode_write step
    // may have reallocated cache->blocks.
    entry = &cache->blocks[blockIndex];
    move(&bytecode, &entry->bytecode);
}

int stack_bytecode_create_empty_entry(Stack* stack, Block* block)
{
    if (stack == NULL)
        return -1;

    BytecodeCache* cache = &stack->bytecode;
    int existing = stack_bytecode_get_index(stack, block);
    if (existing != -1)
        return existing;

    // Doesn't exist, new create entry.
    int newIndex = cache->blockCount++;
    cache->blocks = (BytecodeCache::BlockEntry*) realloc(cache->blocks,
        sizeof(*cache->blocks) * cache->blockCount);

    BytecodeCache::BlockEntry* entry = &cache->blocks[newIndex];
    initialize_null(&entry->bytecode);
    entry->block = block;

    Value key;
    set_block(&key, block);
    set_int(hashtable_insert(&cache->indexMap, &key), newIndex);

    return newIndex;
}

int stack_bytecode_create_entry(Stack* stack, Block* block)
{
    int index = stack_bytecode_create_empty_entry(stack, block);
    stack_bytecode_generate_bytecode(stack, index);
    return index;
}

void stack_bytecode_start_run(Stack* stack)
{
    // Extract the current hackset
    Value hackset;
    stack_derive_hackset(stack, &hackset);

    if (strict_equals(&stack->currentHackset, &hackset))
        return;

    // Hackset has changed.
    stack_bytecode_erase(stack);
    move(&hackset, &stack->currentHackset);
}

void stack_bytecode_erase(Stack* stack)
{
    BytecodeCache* cache = &stack->bytecode;

    for (int b=0; b < cache->blockCount; b++) {
        BytecodeCache::BlockEntry* blockEntry = &cache->blocks[b];
        set_null(&blockEntry->bytecode);
    }
    free(cache->blocks);
    cache->blocks = NULL;
    cache->blockCount = 0;
    set_hashtable(&cache->indexMap);

    for (int i=0; i < stack->frames.count; i++) {
        stack->frames.frame[i].bc = NULL;
        stack->frames.frame[i].pc = 0;
        stack->frames.frame[i].blockIndex = -1;
    }
    stack->bc = NULL;
    stack->pc = 0;
}

void stack_derive_hackset(Stack* stack, Value* hackset)
{
    set_list(hackset);

    if (as_bool_opt(hashtable_get_symbol_key(&stack->env, sym_vmNoEffect), false))
        set_symbol(list_append(hackset), sym_vmNoEffect);
    if (as_bool_opt(hashtable_get_symbol_key(&stack->env, sym_vmNoSaveState), false))
        set_symbol(list_append(hackset), sym_vmNoSaveState);
}

caValue* stack_demand_value_insert(Stack* stack, Term* term)
{
    Value key;
    set_term_ref(&key, term);
    return hashtable_insert(&stack->demandValues, &key);
}

caValue* stack_demand_value_get(Stack* stack, Term* term)
{
    Value key;
    set_term_ref(&key, term);
    return hashtable_get(&stack->demandValues, &key);
}

void stack_on_migration(Stack* stack)
{
    set_hashtable(&stack->demandValues);
    stack_bytecode_erase(stack);
}

Stack* frame_ref_get_stack(caValue* value)
{
    return as_stack(list_get(value, 0));
}

int frame_ref_get_index(caValue* value)
{
    return as_int(list_get(value, 1));
}

Frame* as_frame_ref(caValue* value)
{
    ca_assert(value->value_type == TYPES.frame);

    Stack* stack = frame_ref_get_stack(value);
    int index = frame_ref_get_index(value);
    if (index >= stack->frames.count)
        return NULL;

    return frame_by_index(stack, index);
}

bool is_frame_ref(caValue* value)
{
    return value->value_type == TYPES.frame;
}

void set_frame_ref(caValue* value, Frame* frame)
{
    set_list(value, 2);
    set_stack(list_get(value, 0), frame->stack);
    set_int(list_get(value, 1), frame_get_index(frame));
    cast(value, TYPES.frame);
}

void set_retained_frame(caValue* frame)
{
    make(TYPES.retained_frame, frame);
}
bool is_retained_frame(caValue* frame)
{
    return frame->value_type == TYPES.retained_frame;
}
caValue* retained_frame_get_block(caValue* frame)
{
    ca_assert(is_retained_frame(frame));
    return list_get(frame, 0);
}
caValue* retained_frame_get_state(caValue* frame)
{
    ca_assert(is_retained_frame(frame));
    return list_get(frame, 1);
}

void copy_stack_frame_outgoing_state_to_retained(Frame* source, caValue* retainedFrame)
{
    if (!is_retained_frame(retainedFrame))
        set_retained_frame(retainedFrame);
    touch(retainedFrame);

    set_block(retained_frame_get_block(retainedFrame), source->block);
    set_value(retained_frame_get_state(retainedFrame), &source->outgoingState);
}

void frame_copy(Frame* left, Frame* right)
{
    right->parentIndex = left->parentIndex;
    copy(&left->registers, &right->registers);
    touch(&right->registers);
    copy(&left->state, &right->state);
    copy(&left->bindings, &right->bindings);
    copy(&left->dynamicScope, &right->dynamicScope);
    right->block = left->block;
    right->termIndex = left->termIndex;
    right->bc = NULL;
    right->pc = left->pc;
    right->callType = left->callType;
    right->exitType = left->exitType;
}

void stack_value_copy(Type*, caValue* source, caValue* dest)
{
    Stack* stack = (Stack*) source->value_data.ptr;
    stack_incref(stack);
    make_no_initialize(source->value_type, dest);
    dest->value_data.ptr = stack;
}

void stack_value_release(caValue* value)
{
    Stack* stack = (Stack*) value->value_data.ptr;
    stack_decref(stack);
}

void stack_setup_type(Type* type)
{
    type->initialize = NULL;
    type->release = stack_value_release;
    type->copy = stack_value_copy;
}

} // namespace circa
