// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "common_headers.h"

namespace circa {

namespace list_t {
    struct ListData;

    TaggedValue* append(TaggedValue* list);
    
    void setup_type(Type*);
    void postponed_setup_type(Type*);
}

// C++ wrapper over list_t
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

    List(List const& copy);
    List const& operator=(List const& rhs);
};

}

#endif
