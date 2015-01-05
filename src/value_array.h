// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct ValueArray {
    int size;
    Value* items;

    void init();
    void reserve(int newSize);
    void growBy(int delta) { reserve(size + delta); }
    void clear();
    Value* operator[](int index);
    Value* last() { return operator[](size-1); }
};

} // namespace circa
