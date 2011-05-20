// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

namespace circa {
namespace handle_t {

    typedef void (*OnRelease)(Value* data);

    void set(Value* container, Type* type, Value* data);
    void set(Value* container, Type* type, void* opaquePointer);
    Value* get(Value* value);
    void* get_ptr(Value* value);
    void setup_type(Type* type);

    template <typename T>
    T* create(Value* value, Type* type)
    {
        T* obj = new T();
        set(value, type, obj);
        return obj;
    }

    template <typename T>
    void templated_release_func(Value* value)
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
