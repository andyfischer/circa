// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "tagged_value.h"

#pragma once

namespace circa {
namespace intrusive_refcounted {

    template <typename T>
    void initialize(Type* type, TaggedValue* value)
    {
        T* instance = new T();
        instance->_refCount = 1;
        value->value_data.ptr = instance;
    }

    template <typename T>
    void release(TaggedValue* value)
    {
        T* instance = (T*) value->value_data.ptr;
        instance->_refCount--;
        if (instance->_refCount <= 0)
            delete instance;
    }

    template <typename T>
    void set(TaggedValue* tv, Type* type, T* value)
    {
        set_null(tv);
        tv->value_type = type;
        tv->value_data = value;
        value->_refCount++;
    }

    template <typename T>
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        set_null(dest);
        dest->value_data = source->value_data;
        dest->value_type = source->value_type;
        ((T*) dest->value_data.ptr)->_refCount++;
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
