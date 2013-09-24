// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "interpreter.h"
#include "kernel.h"
#include "stack.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

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
    if (index >= stack->framesCount)
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
Block* retained_frame_get_block(caValue* frame)
{
    ca_assert(is_retained_frame(frame));
    return as_block(list_get(frame, 1));
}
caValue* retained_frame_get_state(caValue* frame)
{
    ca_assert(is_retained_frame(frame));
    return list_get(frame, 2);
}

void copy_stack_frame_outgoing_state_to_retained(Frame* source, caValue* retainedFrame)
{
    if (!is_retained_frame(retainedFrame))
        set_retained_frame(retainedFrame);
    touch(retainedFrame);

    set_stack(list_get(retainedFrame, 0), source->stack);
    set_block(list_get(retainedFrame, 1), source->block);
    set_value(list_get(retainedFrame, 2), &source->outgoingState);
}

void frame_copy(Frame* left, Frame* right)
{
    right->parentPc = left->parentPc;
    copy(&left->registers, &right->registers);
    touch(&right->registers);
    copy(&left->state, &right->state);
    copy(&left->customBytecode, &right->customBytecode);
    copy(&left->dynamicScope, &right->dynamicScope);
    right->block = left->block;
    right->pcIndex = left->pcIndex;
    right->pc = left->pc;
    right->callType = left->callType;
    right->exitType = left->exitType;
}

} // namespace circa
