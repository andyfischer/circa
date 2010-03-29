// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_LIST_T_INCLUDED
#define CIRCA_LIST_T_INCLUDED

#include "common_headers.h"

#include "tagged_value.h"

namespace circa {

namespace list_t {
    struct ListData;

    TaggedValue* append(TaggedValue* list);
    void resize(TaggedValue* list, int newSize);

    void setup_type(Type*);
    void postponed_setup_type(Type*);
}

struct List : TaggedValue
{
    List();

    TaggedValue* append();
    void clear();
    int length();
    TaggedValue* operator[](int index);
    void resize(int size);
};

}

#endif
