// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"
#include "term.h"

namespace circa {

struct ValueIterator
{
    struct IteratorFrame {
        Value* value;
        int index;

        IteratorFrame(Value* v, int i) : value(v), index(i) {}
    };

    std::vector<IteratorFrame> _stack;

    ValueIterator(Value* value)
    {
        if (is_list(value)) {
            _stack.push_back(IteratorFrame(value, 0));
            advanceWhileInvalid();
        }
    }
    bool finished()
    {
        return _stack.empty();
    }
    bool unfinished() { return !finished(); }
    Value* current()
    {
        IteratorFrame& frame = _stack.back();
        return list_get(_stack.back().value, frame.index);
    }
    void advance()
    {
        Value* c = current();

        if (is_list(c) && list_length(c) > 0) {
            _stack.push_back(IteratorFrame(c, 0));
            return;
        }

        IteratorFrame& frame = _stack.back();
        frame.index++;
        advanceWhileInvalid();
    }
    void advanceWhileInvalid()
    {
        while (true) {
            if (_stack.size() == 0)
                return;

            IteratorFrame& frame = _stack.back();

            if (frame.index >= list_length(frame.value)) {
                _stack.pop_back();

                if (_stack.size() > 0)
                    _stack.back().index++;

                continue;
            }

            return;
        }
    }

    Value* operator*() { return current(); }
    void operator++() { advance(); }
};

} // namespace circa
