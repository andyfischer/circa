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
    Type::Release releaseFunc;
};

HandleContainer* alloc_handle_container(Type::Release releaseFunc)
{
    HandleContainer* c = (HandleContainer*) malloc(sizeof(HandleContainer));
    c->refcount = 1;
    c->releaseFunc = releaseFunc;
    set_null(&c->value);
    return c;
}

HandleContainer* get_handle_container(TValue* handle)
{
    if (handle->value_type->storageType != STORAGE_TYPE_HANDLE)
        return NULL;

    return (HandleContainer*) handle->value_data.ptr;
}

void handle_release(Type* type, TValue* value)
{
    HandleContainer* container = get_handle_container(value);
    ca_assert(container != NULL);

    container->refcount--;
    if (container->refcount <= 0) {
        if (container->releaseFunc != NULL)
            container->releaseFunc(NULL, &container->value);
        free(container);
        set_null(value);
    }
}

void setup_handle_type(Type* type)
{
    type->storageType = STORAGE_TYPE_HANDLE;
    type->initialize = NULL;
    type->release = handle_release;
}


void set_handle_value(TValue* handle, TValue* value)
{
    
}

} // namespace circa
