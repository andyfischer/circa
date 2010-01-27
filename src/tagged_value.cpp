// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "type.h"

namespace circa {

TaggedValue::TaggedValue()
    : type(NULL)
{
    type = NULL_T;
    data.ptr = 0;
}

TaggedValue::~TaggedValue()
{
    // deallocate this value
    change_type(*this, NULL_T);
}

TaggedValue::TaggedValue(TaggedValue const& copy)
{
    assert(false);
}

TaggedValue&
TaggedValue::operator=(TaggedValue const& rhs)
{
    assert(false);
    return *this;
}

}
