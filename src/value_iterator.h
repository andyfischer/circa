// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"
#include "term.h"

namespace circa {

struct ValueIterator
{
    struct IteratorFrame {
        caValue* value;
        int index;

        IteratorFrame(caValue* v, int i) : value(v), index(i) {}
    };

    std::vector<IteratorFrame> _stack;

    ValueIterator(caValue* value)
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
    caValue* current()
    {
        IteratorFrame& frame = _stack.back();
        return _stack.back().value->getIndex(frame.index);
    }
    void advance()
    {
        caValue* c = current();

        if (is_list(c)) {
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

    caValue* operator*() { return current(); }
    void operator++() { advance(); }
};

} // namespace circa
