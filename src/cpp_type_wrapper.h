// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "tagged_value.h"

#pragma once

namespace circa {
namespace cpp_type_wrapper {

    template <typename T>
    T* get(TaggedValue* tv)
    {
        return (T*) tv->value_data.ptr;
    }

    template <typename T>
    std::string toString(TaggedValue* tv)
    {
        return get<T>(tv)->toString();
    }

} // namespace type_wrapper
} // namespace circa
