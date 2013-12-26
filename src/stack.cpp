// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "hashtable.h"
#include "inspection.h"
#include "interpreter.h"
#include "kernel.h"
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

    step = sym_StackReady;
    frames.capacity = 0;
    frames.count = 0;
    frames.frame = NULL;
    nextStack = NULL;
    prevStack = NULL;
    
    set_hashtable(&moduleFrames);
    rand_init(&randState, 0);
}

Stack::~Stack()
{
    // Clear error, so that stack_pop doesn't complain about losing an errored frame.
    stack_ignore_error(this);

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
        initialize_null(&frame->customBytecode);
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
    return dupe;
}

caValue* stack_active_value_for_block_id(Frame* frame, int blockId, int termIndex)
{
    Stack* stack = frame->stack;

    while (true) {
        if (frame->block->id == blockId) {
            Term* term = frame_term(frame, termIndex);
            if (is_value(term))
                return term_value(term);

            return frame_register(frame, termIndex);
        }

        frame = frame_parent(frame);

        if (frame == NULL)
            break;
    }

    // Check moduleFrames.
    caValue* moduleFrame = stack_module_frame_get(stack, blockId);
    if (moduleFrame != NULL)
        return list_get(module_frame_get_registers(moduleFrame), termIndex);

    return NULL;
}

caValue* stack_active_value_for_term(Frame* frame, Term* term)
{
    return stack_active_value_for_block_id(frame, term->owningBlock->id, term->index);
}

caValue* stack_module_frame_get(Stack* stack, int blockId)
{
    return hashtable_get_int_key(&stack->moduleFrames, blockId);
}

caValue* stack_module_frame_save(Stack* stack, Block* block, caValue* registers)
{
    caValue* val = hashtable_insert_int_key(&stack->moduleFrames, block->id);
    make(TYPES.retained_frame, val);
    set_block(list_get(val, 0), block);
    set_value(list_get(val, 1), registers);
    return val;
}

Block* module_frame_get_block(caValue* moduleFrame)
{
    return as_block(list_get(moduleFrame, 0));
}

caValue* module_frame_get_registers(caValue* moduleFrame)
{
    return list_get(moduleFrame, 1);
}

void stack_on_migration(Stack* stack)
{
    set_hashtable(&stack->moduleFrames);
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
    make(TYPES.frame, value);
    set_stack(list_get(value, 0), frame->stack);
    set_int(list_get(value, 1), frame_get_index(frame));
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
    copy(&left->customBytecode, &right->customBytecode);
    copy(&left->bindings, &right->bindings);
    copy(&left->dynamicScope, &right->dynamicScope);
    right->block = left->block;
    right->termIndex = left->termIndex;
    right->pc = left->pc;
    right->callType = left->callType;
    right->exitType = left->exitType;
}

} // namespace circa
