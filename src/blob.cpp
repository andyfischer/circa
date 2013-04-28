// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "kernel.h"
#include "tagged_value.h"

namespace circa {

struct BlobData {
    int refCount;
    int length;
    char data[0];
};

void incref(BlobData* data)
{
    data->refCount++;
}

void decref(BlobData* data)
{
    ca_assert(data->refCount > 0);
    data->refCount--;
    if (data->refCount == 0) {
        free(data);
    }
}

BlobData* blob_create(int length)
{
    BlobData* result = (BlobData*) malloc(sizeof(BlobData) + length);
    result->refCount = 1;
    result->length = length;
    return result;
}

BlobData* blob_duplicate(BlobData* original)
{
    BlobData* dupe = blob_create(dupe->length);
    memcpy(dupe->data, original->data, original->length);
    return dupe;
}

void blob_touch(BlobData** blob)
{
    ca_assert((*blob)->refCount > 0);
    if ((*blob)->refCount == 1)
        return;

    BlobData* dup = blob_duplicate(*blob);
    decref(*blob);
    *blob = dup;
}

void blob_resize(BlobData** existing, int length)
{
    if (*existing == NULL) {
        *existing = blob_create(length);
        return;
    }

    if ((*existing)->refCount == 1) {
        *existing = (BlobData*) realloc(*existing, sizeof(BlobData) + length);
        (*existing)->length = length;
        return;
    }

    BlobData* resized = blob_create(length);
    memcpy(resized->data, (*existing)->data, std::min(length, (*existing)->length));
    decref(*existing);
    *existing = resized;
}

int blob_size(BlobData* data)
{
    return data->length;
}

int blob_size(caValue* blob)
{
    ca_assert(is_blob(blob));
    return blob_size((BlobData*) blob->value_data.ptr);
}

void blob_resize(caValue* blob, int size)
{
    ca_assert(is_blob(blob));
    blob_resize((BlobData**) &blob->value_data.ptr, size);
}

void blob_append_char(caValue* blob, char c)
{
    int size = blob_size(blob);
    blob_resize(blob, size + 1);
    as_blob(blob)[size - 1] = c;
}

void blob_append_int(caValue* blob, int i)
{
    int size = blob_size(blob);
    blob_resize(blob, size + 4);
    int* position = (int*) &as_blob(blob)[size - 4];
    *position = i;
}

bool is_blob(caValue* value)
{
    return value->value_type == TYPES.blob;
}

char* as_blob(caValue* value)
{
    ca_assert(is_blob(value));
    return ((BlobData*) value->value_data.ptr)->data;
}

void set_blob(caValue* value, int length)
{
    change_type(value, TYPES.blob);
    value->value_data.ptr = blob_create(length);
}

} // namespace circa
