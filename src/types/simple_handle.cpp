// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

#include "simple_handle.h"

namespace circa {
namespace simple_handle_t {

    void set(Type* type, TaggedValue* value, int handle)
    {
        set_null(value);
        change_type_no_initialize(value, type);
        value->value_data.ptr = allocate_list(1);
        set_int(list_get_element(value, 0), handle);
    }

    void initialize(Type* type, TaggedValue* value)
    {
        internal_error("don't call simple_handle::initialize");
    }
    void release(Type* type, TaggedValue* value)
    {
        ListData* data = (ListData*) value->value_data.ptr;

        ca_assert(data->refCount > 0);

        data->refCount--;

        if (data->refCount == 0) {

            int handle = as_int(list_get_element(value, 0));
            OnRelease onReleaseFunc = (OnRelease) as_opaque_pointer(&type->parameter);

            onReleaseFunc(handle);
            
            free_list(data);
            value->value_data.ptr = NULL;
        }
    }
    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type_no_initialize(dest, type);
        dest->value_data = source->value_data;

        ListData* data = (ListData*) source->value_data.ptr;

        ca_assert(data->refCount > 0);

        data->refCount++;
    }

    void setup_type(Type* type)
    {
        type->name = "simple_handle";
        type->initialize = initialize;
        type->release = release;
        type->copy = copy;
        type->storageType = STORAGE_TYPE_LIST;
    }
}
}
