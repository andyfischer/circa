// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "../tagged_value.h"

namespace circa {

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, TValue*);
    std::string tv_to_string(TValue* value);
    TValue* append(TValue* list);
    TValue* prepend(TValue* list);
}

bool is_list_based_type(Type*);

// Wrapper type to use a TValue as a List.
struct List : TValue
{
    List();

    TValue* append();
    TValue* prepend();
    void append(TValue* val);
    TValue* insert(int index);
    void clear();
    int length();
    bool empty();
    TValue* get(int index);
    void set(int index, TValue* value);
    TValue* operator[](int index) { return get(index); }
    void resize(int size);

    // get the item at length - 1
    TValue* getLast();

    // remove the item at length - 1
    void pop();

    // remove the item at the given index
    void remove(int index);

    void removeNulls();

    // Convenience methods
    void appendString(const char* str);
    void appendString(const std::string& str);

    static List* checkCast(TValue* v);
    static List* lazyCast(TValue* v);
    static List* cast(TValue* v, int length);
};

}
