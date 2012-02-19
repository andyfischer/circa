// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "type.h"
#include "tagged_value.h"

#include "handle.h"

namespace circa {

struct HandleContainer
{
    int refcount;
    TValue value;
    ReleaseFunc releaseFunc;
};

HandleContainer* alloc_handle_container()
{
    HandleContainer* c = (HandleContainer*) malloc(sizeof(HandleContainer));
    c->refcount = 1;
    c->releaseFunc = NULL;
    initialize_null(&c->value);
    return c;
}

HandleContainer* get_handle_container(TValue* handle)
{
    if (handle->value_type->storageType != STORAGE_TYPE_HANDLE)
        return NULL;

    return (HandleContainer*) handle->value_data.ptr;
}

TValue* get_handle_value(TValue* handle)
{
    HandleContainer* container = get_handle_container(handle);
    if (container == NULL)
        return NULL;
    return &container->value;
}

void* get_handle_value_opaque_pointer(TValue* handle)
{
    return as_opaque_pointer(get_handle_value(handle));
}

void handle_release(TValue* value)
{
    if (value->value_data.ptr == NULL)
        return;

    HandleContainer* container = get_handle_container(value);
    ca_assert(container != NULL);

    container->refcount--;

    if (container->refcount <= 0) {
        if (container->releaseFunc != NULL)
            container->releaseFunc(&container->value);
        free(container);
    }
}

void handle_copy(Type* type, TValue* source, TValue* dest)
{
    set_null(dest);
    get_handle_container(source)->refcount++;
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
}

void setup_handle_type(Type* type)
{
    type->storageType = STORAGE_TYPE_HANDLE;
    type->initialize = NULL;
    type->copy = handle_copy;
    type->release = handle_release;
}

void set_handle_value(TValue* handle, Type* type, TValue* value, ReleaseFunc releaseFunc)
{
    set_null(handle);
    change_type(handle, type);
    HandleContainer* container = alloc_handle_container();
    swap(value, &container->value);
    container->releaseFunc = releaseFunc;
    handle->value_data.ptr = container;
}

void set_handle_value_opaque_pointer(TValue* handle, Type* type, void* ptr, ReleaseFunc releaseFunc)
{
    TValue pointerVal;
    set_opaque_pointer(&pointerVal, ptr);
    set_handle_value(handle, type, &pointerVal, releaseFunc);
}
void handle_set_release_func(TValue* handle, ReleaseFunc releaseFunc)
{
    HandleContainer* container = get_handle_container(handle);
    container->releaseFunc = releaseFunc;
}

} // namespace circa
