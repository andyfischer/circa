// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "interpreter.h"
#include "function.h"
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

caValue* handle_get_value(caValue* handle)
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

        // Find the type's release function (if any), and call it.
        Term* releaseMethod = find_method(NULL, value->value_type, "release");
        if (releaseMethod != NULL) {
            Stack stack;
            push_frame(&stack, function_contents(releaseMethod));
            caValue* inputSlot = get_input(&stack, 0);

            // Don't copy this value, otherwise we'll get in trouble when the copy
            // needs to be released.
            swap(value, inputSlot);

            run_interpreter(&stack);

            swap(value, inputSlot);
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

bool handle_equals(caValue* left, caValue* right)
{
    if (!is_handle(right))
        return false;

    return left->value_data.ptr == right->value_data.ptr;
}

int handle_hash(caValue* val)
{
    return val->value_data.asint;
}

void setup_handle_type(Type* type)
{
    type->storageType = name_StorageTypeHandle;
    type->initialize = handle_initialize;
    type->copy = handle_copy;
    type->release = handle_release;
    type->equals = handle_equals;
    type->hashFunc = handle_hash;
}

} // namespace circa
