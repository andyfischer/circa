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

#if __cplusplus
protected:
    caValue() {} // Disallow construction of this type.
#endif
};

namespace circa {

struct Value : public caValue
{
    Value()
    {
        circa_init_value(this);
    }
    ~Value()
    {
        circa_set_null(this);
    }
};

} // namespace circa 

#endif
