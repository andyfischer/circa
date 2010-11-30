// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace list_t {
    bool is_list_based_type(Type*);

    void setup_type(Type*);
    void postponed_setup_type(Term*);
    void tv_initialize(Type*, TaggedValue*);
    TaggedValue* append(TaggedValue* list);
    TaggedValue* prepend(TaggedValue* list);
    void remove_and_replace_with_back(TaggedValue* list, int index);

    namespace tests { void register_tests(); }
}

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

    static List* checkCast(TaggedValue* v);
};

}
