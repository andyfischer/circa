// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, TaggedValue*);
    std::string tv_to_string(TaggedValue* value);
    TaggedValue* append(TaggedValue* list);
    TaggedValue* prepend(TaggedValue* list);

    namespace tests { void register_tests(); }
}

bool is_list_based_type(Type*);

// Wrapper type to use a TaggedValue as a List.
struct List : TaggedValue
{
    List();

    TaggedValue* append();
    TaggedValue* prepend();
    void append(TaggedValue* val);
    void clear();
    int length();
    bool empty();
    TaggedValue* get(int index);
    void set(int index, TaggedValue* value);
    TaggedValue* operator[](int index) { return get(index); }
    void resize(int size);

    // get the item at length - 1
    TaggedValue* getLast();

    // remove the item at length - 1
    void pop();

    // remove the item at the given index
    void remove(int index);

    void removeNulls();

    static List* checkCast(TaggedValue* v);
    static List* lazyCast(TaggedValue* v);
    static List* cast(TaggedValue* v, int length);
};

}
