// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#ifndef CIRCA_CPP_H_INCLUDED
#define CIRCA_CPP_H_INCLUDED

#include "./circa.h"

union caValueData {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

struct caValue
{
    caValueData value_data;
    caType* value_type;
};

namespace circa {

struct Value : public caValue
{
    Value()
    {
        circ_init_value(this);
    }
    ~Value()
    {
        circ_set_null(this);
    }
};

} // namespace circa 

#endif
