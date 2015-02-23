// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "builtin_types.h"
#include "kernel.h"
#include "tagged_value.h"
#include "type.h"
#include "native_ptr.h"

namespace circa {

struct NativePtr
{
    int refCount;
    void* ptr;
    caNativePtrRelease release;
};

bool is_native_ptr(Value* value)
{
    return value->value_type == TYPES.native_ptr;
}

void* as_native_ptr(Value* value)
{
    ca_assert(is_native_ptr(value));
    NativePtr* data = (NativePtr*) value->value_data.ptr;
    return data->ptr;
}

void set_native_ptr(Value* value, void* ptr, caNativePtrRelease release)
{
    make_no_initialize(TYPES.native_ptr, value);
    NativePtr* data = new NativePtr();
    data->refCount = 1;
    data->ptr = ptr;
    data->release = release;
    value->value_data.ptr = data;
}

void native_ptr_initialize(Type*, Value* value)
{
    NativePtr* data = new NativePtr();
    data->refCount = 1;
    data->ptr = NULL;
    data->release = NULL;
    value->value_data.ptr = data;
}

void native_ptr_copy(Value* source, Value* dest)
{
    make_no_initialize(source->value_type, dest);

    NativePtr* data = (NativePtr*) source->value_data.ptr;
    data->refCount++;
    dest->value_data.ptr = data;
}

void native_ptr_release(Value* value)
{
    NativePtr* data = (NativePtr*) value->value_data.ptr;
    data->refCount--;
    if (data->refCount <= 0) {
        if (data->release != NULL)
            data->release(data->ptr);
        delete data;
        value->value_data.ptr = NULL;
    }
}

void native_ptr_setup_type(Type* type)
{
    set_string(&type->name, "native_ptr");
    type->initialize = native_ptr_initialize;
    type->copy = native_ptr_copy;
    type->release = native_ptr_release;
    type->storageType = s_StorageTypeOpaquePointer;
    type->hashFunc = shallow_hash_func;
}

CIRCA_EXPORT void* circa_native_ptr(Value* val)
{
    return as_native_ptr(val);
}

CIRCA_EXPORT void circa_set_native_ptr(Value* val, void* ptr, caNativePtrRelease release)
{
    set_native_ptr(val, ptr, release);
}

CIRCA_EXPORT void circa_set_boxed_native_ptr(Value* val, void* ptr, caNativePtrRelease release)
{
    set_list(val, 1);
    set_native_ptr(val->index(0), ptr, release);
}

} // namespace circa
