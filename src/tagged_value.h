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
    TaggedValueData value_data;
    Type* value_type;

    TaggedValue();
    ~TaggedValue();
    TaggedValue(TaggedValue const&);
    TaggedValue(Type* type);
    TaggedValue& operator=(TaggedValue const& rhs);

    void init();

    void reset();
    std::string toString();
    TaggedValue* operator[](int index);
    int numElements();
};

} // namespace circa

#endif
