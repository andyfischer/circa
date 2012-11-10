// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "type.h"
#include "tagged_value.h"
#include "type.h"

#include "handle.h"

namespace circa {

struct HandleData
{
    int refcount;
    caValue value;
};

bool is_handle(caValue* value)
{
    return value->value_type->storageType == name_StorageTypeHandle;
}

HandleData* as_handle(caValue* handle)
{
    ca_assert(handle->value_type->storageType == name_StorageTypeHandle);
    return (HandleData*) handle->value_data.ptr;
}

caValue* get_handle_value(caValue* handle)
{
    HandleData* container = as_handle(handle);
    return &container->value;
}

void handle_initialize(Type* type, caValue* value)
{
    HandleData* c = (HandleData*) malloc(sizeof(HandleData));
    c->refcount = 1;
    initialize_null(&c->value);
    value->value_data.ptr = c;
}

void handle_release(caValue* value)
{
    HandleData* container = as_handle(value);
    ca_assert(container != NULL);

    container->refcount--;

    // Release data, if this is the last reference.
    if (container->refcount <= 0) {

        // Find the type's custom release func (if defined).
        caValue* releaseFunc = get_type_property(value->value_type, "handle.release");

        if (releaseFunc != NULL && is_opaque_pointer(releaseFunc)) {
            ReleaseFunc func = (ReleaseFunc) as_opaque_pointer(releaseFunc);
            func(&container->value);
        }

        free(container);
    }
}

void handle_copy(Type* type, caValue* source, caValue* dest)
{
    set_null(dest);

    as_handle(source)->refcount++;
    dest->value_type = source->value_type;
    dest->value_data.ptr = source->value_data.ptr;
}

void setup_handle_type(Type* type)
{
    type->storageType = name_StorageTypeHandle;
    type->initialize = handle_initialize;
    type->copy = handle_copy;
    type->release = handle_release;
}

void handle_type_set_release_func(Type* type, ReleaseFunc releaseFunc)
{
    set_opaque_pointer(type_property_insert(type, "handle.release"), (void*) releaseFunc);
}

/*
void* get_handle_value_opaque_pointer(caValue* handle)
{
    return as_opaque_pointer(get_handle_value(handle));
}

void set_handle_value_opaque_pointer(caValue* handle, Type* type, void* ptr, ReleaseFunc releaseFunc)
{
    Value pointerVal;
    set_opaque_pointer(&pointerVal, ptr);
    move(&pointerVal, set_handle_value(handle, type, releaseFunc));
}
void handle_set_release_func(caValue* handle, ReleaseFunc releaseFunc)
{
    HandleData* container = as_handle(handle);
    container->releaseFunc = releaseFunc;
}
*/

} // namespace circa
