// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "block.h"
#include "change_events.h"
#include "interpreter.h"
#include "list.h"
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
    Frame* frame;

    WorldFramesIterator(World* world)
      : stacksIterator(world)
    {
        frame = first_frame(stacksIterator.current());
        advanceWhileInvalid();
    }

    void advanceWhileInvalid()
    {
    still_invalid:
        if (finished())
            return;

        if (frame == NULL) {
            ++stacksIterator;
            frame = first_frame(stacksIterator.current());
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
        return frame;
    }

    void advance()
    {
        frame = next_frame(frame);
        advanceWhileInvalid();
    }

    operator bool()
    {
        return !finished();
    }

    void operator++() { advance(); }
};

Value* change_event_type(Value* event)
{
    return list_get(event, 0);
}
Value* change_event_attrs(Value* event)
{
    return list_get(event, 1);
}
Value* change_event_target(Value* event)
{
    return list_get(event, 2);
}

static Block* get_single_block_target(Value* target)
{
    // Future: This might support alternate ways of indicating a block.
    return as_block(target);
}

Value* change_event_field(Value* event, int index)
{
    return list_get(event, 3 + index);
}

void change_event_set_blank(Value* value, int eventSpecificFields)
{
    set_list(value, 3 + eventSpecificFields);
    set_symbol(change_event_type(value), s_none);
    set_hashtable(change_event_attrs(value));
}

void change_event_make_rename(Value* event, Term* target, const char* newName)
{
}

void change_event_make_append(Value* event, Block* target, Value* expression)
{
    change_event_set_blank(event, 1);
    set_symbol(change_event_type(event), s_ChangeAppend);
    set_block(change_event_target(event), target);
    set_value(change_event_field(event, 0), expression);
}

static void commit_append(caWorld* world, Value* event, bool dryRun, Value* result)
{
    // Always succeeds.
    set_bool(result, true);
    if (dryRun)
        return;

    Block* block = get_single_block_target(change_event_target(event));

    compile(block, as_cstring(change_event_field(event, 0)));
}

void change_event_commit(caWorld* world, Value* event, bool dryRun, Value* result)
{
    Symbol type = as_symbol(change_event_type(event));

    switch (type) {
    case s_ChangeAppend:
        commit_append(world, event, dryRun, result);
        break;
    default:
        internal_error("Unknown change event type in change_event_commit");
    }
}

void change_event_save_inverse(Value* world, Value* event, Value* inverse)
{
    // TODO
}

} // namespace circa
