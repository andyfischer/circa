// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_LIST_INCLUDED
#define CIRCA_LIST_INCLUDED

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

struct List
{
    std::vector<TaggedValue> _items;

    TaggedValue* append();
    TaggedValue* append(Type* type);
    TaggedValue* get(int index);
    TaggedValue* operator[](int index);
};

#if 0
bool is_list(TaggedValue*);
List& as_list(TaggedValue*);
#endif

}

#endif
