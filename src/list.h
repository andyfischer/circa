// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "common_headers.h"

namespace circa {

    namespace list_t {
        struct ListData;
    }

struct List
{
    list_t::ListData* _data;

    int length() const;

    // Append an element on the end, and returns the new element.
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
