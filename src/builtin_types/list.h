// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "../tagged_value.h"

namespace circa {

namespace list_t {
    bool is_list(TaggedValue*);
    bool is_list_based_type(Type*);
    void setup_type(Type*);
    void postponed_setup_type(Term*);
    void tv_initialize(Type*, TaggedValue*);
    void remove_and_replace_with_back(TaggedValue* list, int index);
}

struct List : TaggedValue
{
    List();

    TaggedValue* append();
    void clear();
    int length();
    TaggedValue* get(int index);
    void set(int index, TaggedValue* value);
    TaggedValue* operator[](int index) { return get(index); }
    void resize(int size);
};

}
