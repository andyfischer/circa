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
    int refCount;
    caValue value;
};

bool is_handle(caValue* value)
{
    return value->value_type->storageType == sym_StorageTypeHandle;
}

HandleData* as_handle(caValue* handle)
{
    ca_assert(handle->value_type->storageType == sym_StorageTypeHandle);
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
    c->refCount = 1;
    initialize_null(&c->value);
    value->value_data.ptr = c;
}

void handle_release(caValue* value)
{
    HandleData* container = as_handle(value);
    ca_assert(container != NULL);
    ca_assert(container->refCount > 0);

    container->refCount--;

    // Release data, if this is the last reference.
    if (container->refCount == 0) {

        // Find the type's release function (if any), and call it.
        Value releaseStr;
        set_string(&releaseStr, "release");
        Term* releaseMethod = find_method(NULL, value->value_type, &releaseStr);
        if (releaseMethod != NULL) {
            Stack stack;
            stack_init(&stack, function_contents(releaseMethod));
            caValue* inputSlot = get_input(&stack, 0);

            // Don't copy this value, otherwise we'll get in trouble when the copy
            // needs to be released.
            swap(value, inputSlot);

            stack_run(&stack);

            inputSlot = get_input(&stack, 0);
            swap(value, inputSlot);
        }

        free(container);
    }
}

void handle_copy(Type*, caValue* source, caValue* dest)
{
    as_handle(source)->refCount++;

    set_null(dest);
    make_no_initialize(source->value_type, dest);
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
    type->storageType = sym_StorageTypeHandle;
    type->initialize = handle_initialize;
    type->copy = handle_copy;
    type->release = handle_release;
    type->equals = handle_equals;
    type->hashFunc = handle_hash;
}

} // namespace circa
