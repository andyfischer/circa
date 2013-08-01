// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "interpreter.h"
#include "kernel.h"
#include "stack.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

Frame* as_frame(caValue* value)
{
    ca_assert(value->value_type == TYPES.frame);
    return (Frame*) value->value_data.ptr;
}

bool is_frame(caValue* value)
{
    return value->value_type == TYPES.frame;
}

void set_frame(caValue* value)
{
    make_no_initialize(TYPES.frame, value);
    value->value_data.ptr = new Frame;
}

void frame_initialize(Type* type, caValue* value)
{
    set_frame(value);
}

void frame_release(caValue* value)
{
    delete as_frame(value);
}

void frame_copy(Frame* left, Frame* right)
{
    right->parentPc = left->parentPc;
    right->role = left->role;
    copy(&left->registers, &right->registers);
    copy(&left->state, &right->state);
    copy(&left->customBytecode, &right->customBytecode);
    copy(&left->dynamicScope, &right->dynamicScope);
    right->block = left->block;
    right->blockVersion = left->blockVersion;
    right->pc = left->pc;
    right->pos = left->pos;
    right->callType = left->callType;
    right->exitType = left->exitType;
    right->retain = left->retain;
}

void frame_copy_boxed(Type*, caValue* leftValue, caValue* rightValue)
{
    Frame* left = as_frame(leftValue);
    set_frame(rightValue);
    Frame* right = as_frame(rightValue);
    frame_copy(left, right);
    touch(&right->registers);
}

void copy_stack_frame_to_boxed(Frame* frame, caValue* value)
{
    set_frame(value);
    frame_copy(frame, as_frame(value));
}

void frame_setup_type(Type* type)
{
    reset_type(type);
    set_string(&type->name, "Frame");
    type->initialize = frame_initialize;
    type->release = frame_release;
    type->copy = frame_copy_boxed;
}

} // namespace circa
