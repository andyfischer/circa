// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "type.h"
#include "tagged_value.h"

#include "handle.h"

namespace circa {

struct HandleContainer
{
    int refcount;
    caValue value;
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

HandleContainer* get_handle_container(caValue* handle)
{
    if (handle->value_type->storageType != name_StorageTypeHandle)
        return NULL;

    return (HandleContainer*) handle->value_data.ptr;
}

caValue* get_handle_value(caValue* handle)
{
    HandleContainer* container = get_handle_container(handle);
    if (container == NULL)
        return NULL;
    return &container->value;
}

void* get_handle_value_opaque_pointer(caValue* handle)
{
    return as_opaque_pointer(get_handle_value(handle));
}

void handle_initialize(Type* type, caValue* value)
{
    value->value_data.ptr = alloc_handle_container();
}

void handle_release(caValue* value)
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

void handle_copy(Type* type, caValue* source, caValue* dest)
{
    set_null(dest);
    get_handle_container(source)->refcount++;
    dest->value_type = source->value_type;
    dest->value_data = source->value_data;
}

void setup_handle_type(Type* type)
{
    type->storageType = name_StorageTypeHandle;
    type->initialize = handle_initialize;
    type->copy = handle_copy;
    type->release = handle_release;
}

void set_handle_value(caValue* handle, Type* type, caValue* value, ReleaseFunc releaseFunc)
{
    set_null(handle);
    change_type(handle, type);
    HandleContainer* container = alloc_handle_container();
    swap(value, &container->value);
    container->releaseFunc = releaseFunc;
    handle->value_data.ptr = container;
}

void set_handle_value(caValue* handle, caValue* value, ReleaseFunc releaseFunc)
{
    HandleContainer* container = get_handle_container(handle);
    swap(value, &container->value);
    container->releaseFunc = releaseFunc;
}

void set_handle_value_opaque_pointer(caValue* handle, Type* type, void* ptr, ReleaseFunc releaseFunc)
{
    Value pointerVal;
    set_opaque_pointer(&pointerVal, ptr);
    set_handle_value(handle, type, &pointerVal, releaseFunc);
}
void handle_set_release_func(caValue* handle, ReleaseFunc releaseFunc)
{
    HandleContainer* container = get_handle_container(handle);
    container->releaseFunc = releaseFunc;
}

bool is_handle(caValue* value)
{
    return value->value_type->storageType == name_StorageTypeHandle;
}

caValue* dereference_handle(caValue* value)
{
    while (is_handle(value))
        value = get_handle_value(value);
    return value;
}

} // namespace circa
