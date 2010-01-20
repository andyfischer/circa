// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "tagged_value.h"
#include "type.h"

namespace circa {

TaggedValue::TaggedValue()
    : type(NULL)
{
    data.ptr = 0;
}

TaggedValue::TaggedValue(TaggedValue const& copy)
{
    std::cout << "tv copy" << std::endl;
}

TaggedValue&
TaggedValue::operator=(TaggedValue const& rhs)
{
    assert(false);
    std::cout << "tv operator =" << std::endl;
    return *this;
}

}
