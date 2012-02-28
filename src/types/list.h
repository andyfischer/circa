// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "../tagged_value.h"

namespace circa {

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, caValue*);
    std::string tv_to_string(caValue* value);
    caValue* append(caValue* list);
    caValue* prepend(caValue* list);
}

bool is_list_based_type(Type*);

// Wrapper type to use a caValue as a List.
struct List : caValue
{
    List();

    caValue* append();
    caValue* prepend();
    void append(caValue* val);
    caValue* insert(int index);
    void clear();
    int length();
    bool empty();
    caValue* get(int index);
    void set(int index, caValue* value);
    caValue* operator[](int index) { return get(index); }
    void resize(int size);

    // get the item at length - 1
    caValue* getLast();

    // remove the item at length - 1
    void pop();

    // remove the item at the given index
    void remove(int index);

    void removeNulls();

    // Convenience methods
    void appendString(const char* str);
    void appendString(const std::string& str);

    static List* checkCast(caValue* v);
    static List* lazyCast(caValue* v);
    static List* cast(caValue* v, int length);
};

}
