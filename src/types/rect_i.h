// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Rect_i : Value
{
    void set(int x1, int y1, int x2, int y2);
    static Rect_i* cast(Value* tv);
};

} // namespace circa
