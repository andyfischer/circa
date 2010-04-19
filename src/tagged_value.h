// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

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
    inline TaggedValue* operator[](int index) { return getElement(index); }
    TaggedValue* getElement(int index);
    int numElements();
};

} // namespace circa
