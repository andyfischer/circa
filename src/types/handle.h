// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {
namespace handle_t {

    typedef void (*OnRelease)(TaggedValue* data);

    void set(TaggedValue* container, Type* type, TaggedValue* data);
    void set(TaggedValue* container, Type* type, void* opaquePointer);
    TaggedValue* get(TaggedValue* value);
    void* get_ptr(TaggedValue* value);
    void setup_type(Type* type);

    template <typename T>
    T* create(TaggedValue* value, Type* type)
    {
        T* obj = new T();
        set(value, type, obj);
        return obj;
    }

    template <typename T>
    void templated_release_func(TaggedValue* value)
    {
        T* object = (T*) as_opaque_pointer(value);
        delete object;
    }

    template <typename T>
    void setup_type(Type* type)
    {
        setup_type(type);
        OnRelease callback = templated_release_func<T>;
        set_opaque_pointer(&type->parameter, (void*) callback);
    }

} // namespace handle_t
} // namespace circa
