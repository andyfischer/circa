// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "kernel.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

const int BLOB_SLICE = 1;

struct BlobAbstract {
    int concreteType;
};

struct BlobSlice {
    int concreteType;
    int refcount;
    Value backingValue;
    char* data;
    u32 numBytes;
};

BlobSlice* alloc_blob_slice()
{
    BlobSlice* slice = (BlobSlice*) malloc(sizeof(*slice));
    slice->concreteType = BLOB_SLICE;
    slice->refcount = 1;
    initialize_null(&slice->backingValue);
    slice->data = NULL;
    slice->numBytes = 0;
    return slice;
}

void decref(BlobSlice* slice)
{
    if (slice == NULL)
        return;

    ca_assert(slice->refcount > 0);

    slice->refcount--;
    if (slice->refcount == 0) {
        set_null(&slice->backingValue);
        free(slice);
    }
}

CIRCA_EXPORT void circa_blob_data(Value* blob, char** dataOut, u32* sizeOut)
{
    ca_assert(is_blob(blob));

    if (blob->value_data.ptr == NULL) {
        *dataOut = NULL;
        *sizeOut = 0;
        return;
    }

    BlobSlice* slice = (BlobSlice*) blob->value_data.ptr;
    *dataOut = slice->data;
    *sizeOut = slice->numBytes;
}

void blob_copy(Value* source, Value* dest)
{
    make_no_initialize(TYPES.blob, dest);

    if (source->value_data.ptr != NULL) {
        dest->value_data.ptr = source->value_data.ptr;
        ((BlobSlice*) source->value_data.ptr)->refcount++;
    }
}

void blob_release(Value* value)
{
    if (value->value_data.ptr == NULL)
        return;
    decref((BlobSlice*) value->value_data.ptr);
}

int blob_hash(Value* value)
{
    char* data;
    u32 size;
    blob_data(value, &data, &size);

    // Dumb and simple hash function
    int result = 0;
    int byte = 0;
    for (u32 i=0; i < size; i++) {
        result = result ^ (data[i] << (8 * byte));
        byte = (byte + 1) % 4;
    }
    return result;
}

CIRCA_EXPORT void circa_set_blob_from_backing_value(Value* blob, Value* backingValue, char* data, u32 numBytes)
{
    BlobSlice* slice = alloc_blob_slice();
    copy(backingValue, &slice->backingValue);
    slice->data = data;
    slice->numBytes = numBytes;

    make_no_initialize(TYPES.blob, blob);
    blob->value_data.ptr = slice;
}

void blob_setup_type(Type* type)
{
    set_string(&type->name, "Blob");
    type->copy = blob_copy;
    type->release = blob_release;
    type->hashFunc = blob_hash;
}

} // namespace circa
