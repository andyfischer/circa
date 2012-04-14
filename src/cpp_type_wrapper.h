// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "tagged_value.h"

#pragma once

namespace circa {
namespace cpp_type_wrapper {

    template <typename T>
    T* get(caValue* tv)
    {
        return (T*) tv->value_data.ptr;
    }

    template <typename T>
    std::string toString(caValue* tv)
    {
        return get<T>(tv)->toString();
    }

} // namespace type_wrapper

namespace heap_value_type_wrapper {

    template <typename T>
    void initialize(Type* type, caValue* value)
    {
        value->value_data.ptr = new T();
    }

    template <typename T>
    void release(caValue* value)
    {
        delete (T*) value->value_data.ptr;
    }

    template <typename T>
    void copy(caValue* source, caValue* dest)
    {
        *((T*) dest->value_data.ptr) = *((T*) source->value_data.ptr);
    }

    template <typename T>
    void setup_type(Type* type)
    {
        type->initialize = initialize<T>;
        type->release = release<T>;
        type->copy = copy<T>;
    }

}
} // namespace circa
