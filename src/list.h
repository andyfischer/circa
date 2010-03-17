// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "common_headers.h"

namespace circa {

namespace list_t {
    struct ListData;

    void setup_type(Type*);
}

struct List
{
    list_t::ListData* _data;

    // Returns number of elements.
    int length() const;

    // Append a new element on the end, and returns the new element.
    TaggedValue* append();

    // Clear all elements.
    void clear();

    // Access an element by index.
    TaggedValue* operator[](int index);
    
    List() : _data(NULL) {}
    ~List();
};

}

#endif
