// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "actors.h"
#include "building.h"
#include "hashtable.h"
#include "interpreter.h"
#include "kernel.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"
#include "world.h"


namespace circa {

// Actor system v3
//
// In this scheme, an "actor" is a Block with associated state. Callers can share an
// actor (and thus share its state, indirectly). Callers can 'call' the actor to use
// it. This calling mechanism is syncronous, there's no direct support for async.
// (But callers can simulate async by injecting into queues).

#if 0
Stack* create_actor(World* world, Block* block)
{
    ca_assert(block != NULL);
    Stack* stack = create_stack(world);
    push_frame(stack, block);
    return stack;
}
#endif

bool state_inject(Stack* stack, caValue* name, caValue* value)
{
    caValue* state = stack_get_state(stack);
    Block* block = top_frame(stack)->block;

    // Initialize stateValue if it's currently null.
    if (is_null(state))
        make(block->stateType, state);

    caValue* slot = get_field(state, name, NULL);
    if (slot == NULL)
        return false;

    touch(state);
    copy(value, get_field(state, name, NULL));
    return true;
}

caValue* context_inject(Stack* stack, caValue* name)
{
    Frame* frame = top_frame(stack);

    if (is_null(&frame->dynamicScope))
        set_hashtable(&frame->dynamicScope);

    return hashtable_insert(&frame->dynamicScope, name);
}

} // namespace circa

