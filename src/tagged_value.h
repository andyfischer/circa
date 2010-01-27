// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#ifndef CIRCA_TAGGED_VALUE_INCLUDED
#define CIRCA_TAGGED_VALUE_INCLUDED

namespace circa {

union TaggedValueData {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

struct TaggedValue
{
    TaggedValueData data;
    Type* type;

    TaggedValue();
    ~TaggedValue();
    TaggedValue(TaggedValue const& copy);
    TaggedValue& operator=(TaggedValue const& rhs);
};

} // namespace circa

#endif
