// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, Value*);
    std::string tv_to_string(Value* value);
    Value* append(Value* list);
    Value* prepend(Value* list);
    void remove_and_replace_with_back(Value* list, int index);

    namespace tests { void register_tests(); }
}

bool is_list_based_type(Type*);

// Wrapper type to use a Value as a List.
struct List : Value
{
    List();

    Value* append();
    Value* prepend();
    void append(Value* val);
    void clear();
    int length();
    bool empty();
    Value* get(int index);
    void set(int index, Value* value);
    Value* operator[](int index) { return get(index); }
    void resize(int size);

    // get the item at length - 1
    Value* getLast();

    // remove the item at length - 1
    void pop();

    // remove the item at the given index
    void remove(int index);

    void removeNulls();

    static List* checkCast(Value* v);
    static List* lazyCast(Value* v);
    static List* cast(Value* v, int length);
};

}
