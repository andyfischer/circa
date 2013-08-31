// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "change_events.h"
#include "interpreter.h"
#include "block.h"
#include "hashtable.h"
#include "tagged_value.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

struct WorldStacksIterator
{
    Stack* first;
    Stack* _current;

    WorldStacksIterator(World* world)
    {
        first = world->firstStack;
        _current = first;
    }
    bool finished()
    {
        return _current == NULL;
    }
    operator bool()
    {
        return !finished();
    }
    Stack* current()
    {
        return _current;
    }
    WorldStacksIterator& operator++()
    {
        _current = _current->nextStack;
        if (_current == first)
            _current = NULL;
        return *this;
    }
};

struct WorldFramesIterator
{
    WorldStacksIterator stacksIterator;
    int frameIndex;

    WorldFramesIterator(World* world)
      : stacksIterator(world)
    {
        frameIndex = 0;
        advanceWhileInvalid();
    }

    void advanceWhileInvalid()
    {
    still_invalid:
        if (finished())
            return;

        if (frameIndex >= stack_frame_count(stacksIterator.current())) {
            ++stacksIterator;
            frameIndex = 0;
            goto still_invalid;
        }
    }

    bool finished()
    {
        return stacksIterator.finished();
    }

    Frame* current()
    {
        ca_assert(!finished());
        return frame_by_index(stacksIterator.current(), frameIndex);
    }

    operator bool()
    {
        return !finished();
    }

    WorldFramesIterator& operator++()
    {
        frameIndex++;
        advanceWhileInvalid();
        return *this;
    }
};

caValue* change_event_type(caValue* event)
{
    return list_get(event, 0);
}
caValue* change_event_attrs(caValue* event)
{
    return list_get(event, 1);
}
caValue* change_event_target(caValue* event)
{
    return list_get(event, 2);
}

static Block* get_single_block_target(caValue* target)
{
    // Future: This might support alternate ways of indicating a block.
    return as_block(target);
}

caValue* change_event_field(caValue* event, int index)
{
    return list_get(event, 3 + index);
}

void change_event_set_blank(caValue* value, int eventSpecificFields)
{
    set_list(value, 3 + eventSpecificFields);
    set_symbol(change_event_type(value), sym_None);
    set_hashtable(change_event_attrs(value));
}

void change_event_make_rename(Value* event, Term* target, const char* newName)
{
}

void change_event_make_append(Value* event, Block* target, caValue* expression)
{
    change_event_set_blank(event, 1);
    set_symbol(change_event_type(event), sym_ChangeAppend);
    set_block(change_event_target(event), target);
    set_value(change_event_field(event, 0), expression);
}

static void commit_append(caWorld* world, caValue* event, bool dryRun, caValue* result)
{
    // Always succeeds.
    set_bool(result, true);
    if (dryRun)
        return;

    Block* block = get_single_block_target(change_event_target(event));

    int prevRegisterCount = block_locals_count(block);

    compile(block, as_cstring(change_event_field(event, 0)));
    dirty_bytecode(block);
    refresh_bytecode(block);

    int newRegisterCount = block_locals_count(block);

    // Grow register lists.
    for (WorldFramesIterator it(world); it; ++it) {
        Frame* frame = it.current();
        if (frame->block == block)
            list_resize(&frame->registers, newRegisterCount);
    }
}

void change_event_commit(caWorld* world, caValue* event, bool dryRun, caValue* result)
{
    caName type = as_symbol(change_event_type(event));

    switch (type) {
    case sym_ChangeAppend:
        commit_append(world, event, dryRun, result);
        break;
    default:
        internal_error("Unknown change event type in change_event_commit");
    }
}

void change_event_save_inverse(caValue* world, caValue* event, caValue* inverse)
{
    // TODO
}

} // namespace circa
