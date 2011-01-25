// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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
