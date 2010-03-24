// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "type.h"

namespace circa {

TaggedValue::TaggedValue()
{
    init();
}

void
TaggedValue::init()
{
    value_type = NULL_T;
    value_data.ptr = 0;
}

TaggedValue::~TaggedValue()
{
    // Deallocate this value
    change_type(this, NULL_T);
}

TaggedValue::TaggedValue(TaggedValue const& original)
{
    value_type = NULL_T;
    value_data.ptr = 0;

    Term* source = (Term*) &original;
    copy(source, this);
}

TaggedValue&
TaggedValue::operator=(TaggedValue const& rhs)
{
    Term* source = (Term*) &rhs;
    copy(source, this);
    return *this;
}

TaggedValue::TaggedValue(Type* type)
{
    init();
    change_type(this, type);
}

std::string
TaggedValue::toString()
{
    return to_string(this);
}

TaggedValue*
TaggedValue::operator[](int index)
{
    return get_element(this, index);
}

int
TaggedValue::numElements()
{
    return num_elements(this);
}

}
