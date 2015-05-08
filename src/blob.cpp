// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "blob.h"
#include "kernel.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"
#include "vm.h"

namespace circa {

const int BLOB_ZERO = 1;
const int BLOB_SLICE = 2;
const int BLOB_FLAT = 3;

/*
  Abstract blob - Header common to all blob types.
*/
struct AbstractBlob {
    int type;
    int refcount;
    u32 size;
};

/*
  Flat - a contiguous block of bytes
*/
struct Flat {
    AbstractBlob header;
    char data[0];
};

/*
  Slice - a size-delimited pointer into unowned data. Data should be referenced by 'backingValue'.
*/

struct Slice {
    AbstractBlob header;
    Value backingValue;
    char* data;
};

Slice* alloc_slice()
{
    Slice* slice = (Slice*) ca_realloc(NULL, sizeof(*slice));
    slice->header.type = BLOB_SLICE;
    slice->header.refcount = 1;
    slice->header.size = 0;
    initialize_null(&slice->backingValue);
    slice->data = NULL;
    return slice;
}

Flat* alloc_flat(u32 size)
{
    Flat* flat = (Flat*) ca_realloc(NULL, sizeof(*flat) + size);
    flat->header.type = BLOB_FLAT;
    flat->header.refcount = 1;
    flat->header.size = size;
    //printf("flat alloc: %p %u\n", flat, size);
    return flat;
}

int get_type(AbstractBlob* abstract)
{
    if (abstract == NULL)
        return BLOB_ZERO;
    else
        return abstract->type;
}

Flat* alloc_flat_and_fill(u32 size, const char* initialData, u32 initialDataSize)
{
    ca_assert(initialDataSize <= size);

    Flat* flat = alloc_flat(size);
    memcpy(flat->data, initialData, std::min(size, initialDataSize));
    if (initialDataSize < size)
        memset(flat->data + initialDataSize, 0, size - initialDataSize);
    return flat;
}

void decref(AbstractBlob* blob)
{
    if (blob == NULL)
        return;

    ca_assert(blob->refcount > 0);

    blob->refcount--;

    if (blob->refcount == 0) {
        switch (blob->type) {
        case BLOB_SLICE:
            Slice* slice = (Slice*) blob;
            set_null(&slice->backingValue);
            break;
        }
        free(blob);
    }
}

Flat* blob_resize(AbstractBlob* abstract, u32 newSize)
{
    switch (get_type(abstract)) {
    case BLOB_ZERO:
        return alloc_flat(newSize);

    case BLOB_SLICE: {
        Slice* slice = (Slice*) abstract;
        Flat* flat = alloc_flat_and_fill(newSize, slice->data, slice->header.size);
        decref(abstract);
        return flat;
    }
    case BLOB_FLAT: {
        Flat* flat = (Flat*) abstract;
        if (flat->header.refcount == 1) {
            //printf("flat realloc: %p %u\n", flat, newSize);
            flat = (Flat*) ca_realloc(flat, sizeof(Flat) + newSize);
            flat->header.size = newSize;
            return flat;
        }

        flat = alloc_flat_and_fill(newSize, flat->data, flat->header.size);
        decref(abstract);
        return flat;
    }
    }

    return NULL;
}

char* blob_data_flat(Value* blob)
{
    AbstractBlob* abstract = (AbstractBlob*) blob->value_data.ptr;
    switch (get_type(abstract)) {
    case BLOB_ZERO:
        return NULL;

    case BLOB_SLICE: {
        Slice* slice = (Slice*) abstract;
        return slice->data;
    }
    case BLOB_FLAT: {
        Flat* flat = (Flat*) abstract;
        return flat->data;
    }
    }
    return NULL;
}

CIRCA_EXPORT char* circa_blob(Value* blob)
{
    return blob_data_flat(blob);
}

void blob_data(Value* blobVal, char** dataOut, u32* sizeOut)
{
    ca_assert(is_blob(blobVal));

    if (blobVal->value_data.ptr == NULL) {
        *dataOut = NULL;
        *sizeOut = 0;
        return;
    }

    AbstractBlob* blob = (AbstractBlob*) blobVal->value_data.ptr;

    switch (blob->type) {
    case BLOB_SLICE: {
        Slice* slice = (Slice*) blob;
        *dataOut = slice->data;
        *sizeOut = slice->header.size;
        break;
    }
    case BLOB_FLAT: {
        Flat* flat = (Flat*) blob;
        *dataOut = flat->data;
        *sizeOut = flat->header.size;
        break;
    }
    }
}

void blob_copy(Value* source, Value* dest)
{
    make_no_initialize(TYPES.blob, dest);

    if (source->value_data.ptr != NULL) {
        dest->value_data.ptr = source->value_data.ptr;
        ((AbstractBlob*) source->value_data.ptr)->refcount++;
    }
}

void blob_release(Value* value)
{
    if (value->value_data.ptr == NULL)
        return;
    decref((AbstractBlob*) value->value_data.ptr);
}

void blob_resize(Value* blob, u32 size)
{
    blob->value_data.ptr = blob_resize((AbstractBlob*) blob->value_data.ptr, size);
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

char* blob_touch(Value* blobVal)
{
    AbstractBlob* abstract = (AbstractBlob*) blobVal->value_data.ptr;

    if (abstract == NULL)
        return NULL;

    if (abstract->type == BLOB_FLAT && (abstract->refcount == 1))
        return ((Flat*) abstract)->data;

    stat_increment(BlobDuplicate);
    char* data;
    u32 numBytes;
    blob_data(blobVal, &data, &numBytes);

    Flat* flat = alloc_flat_and_fill(numBytes, data, numBytes);
    decref((AbstractBlob*) blobVal->value_data.ptr);
    blobVal->value_data.ptr = flat;
    return flat->data;
}

u32 blob_size(Value* blob)
{
    AbstractBlob* abstract = (AbstractBlob*) blob->value_data.ptr;
    if (abstract == NULL)
        return 0;
    return abstract->size;
}

CIRCA_EXPORT uint32_t circa_blob_size(Value* blob)
{
    return blob_size(blob);
}


#if 0
void set_blob_from_backing_value(Value* blob, Value* backingValue, char* data, u32 numBytes)
{
    Slice* slice = alloc_slice();
    copy(backingValue, &slice->backingValue);
    slice->data = data;
    slice->numBytes = numBytes;

    make_no_initialize(TYPES.blob, blob);
    blob->value_data.ptr = slice;
}
#endif

void blob_setup_type(Type* type)
{
    set_string(&type->name, "Blob");
    type->copy = blob_copy;
    type->release = blob_release;
    type->hashFunc = blob_hash;
}

void set_blob(Value* value, u32 initialSize)
{
    make_no_initialize(TYPES.blob, value);
    value->value_data.ptr = alloc_flat(initialSize);
}

void set_blob_slice(Value* value, Value* backingValue, const char* data, u32 size)
{
    Slice* slice = alloc_slice();
    if (backingValue != NULL)
        copy(backingValue, &slice->backingValue);
    slice->data = (char*) data;
    slice->header.size = size;

    make_no_initialize(TYPES.blob, value);
    value->value_data.ptr = slice;
}

CIRCA_EXPORT void circa_set_blob_slice(Value* value, Value* backingValue, const char* data, uint32_t size)
{
    set_blob_slice(value, backingValue, data, size);
}

void set_blob_flat(Value* value, const char* data, u32 size)
{
    Flat* flat = alloc_flat_and_fill(size, data, size);
    make_no_initialize(TYPES.blob, value);
    value->value_data.ptr = flat;
}

void set_blob_from_str(Value* value, const char* str)
{
    set_blob_flat(value, str, strlen(str));
}

char* blob_grow(Value* blob, u32 growSize)
{
    AbstractBlob* abstract = (AbstractBlob*) blob->value_data.ptr;
    Flat* flat = blob_resize(abstract, abstract->size + growSize);
    blob->value_data.ptr = flat;
    return flat->data + flat->header.size - growSize;
}

void blob_slice(Value* blob, int start, int end, Value* sliceOut)
{
    if (end == -1)
        end = blob_size(blob);

    AbstractBlob* abstract = (AbstractBlob*) blob->value_data.ptr;

    if (abstract->type != BLOB_FLAT)
        internal_error("blob_slice: unimplemented");

    const char* data = ((Flat*) abstract)->data;

    set_blob_slice(sliceOut, blob, data + start, end - start);
}

void blob_to_str(Value* blob)
{
    AbstractBlob* abstract = (AbstractBlob*) blob->value_data.ptr;

    Value out;
    set_string(&out, blob_data_flat(blob), blob_size(blob));
    move(&out, blob);
}

void make_blob(VM* vm)
{
    u32 size = vm->input(0)->as_i();
    set_blob(vm->output(), size);
}

void Blob__size(VM* vm)
{
    set_int(vm->output(), blob_size(vm->input(0)));
}

void Blob__resize(VM* vm)
{
    Value* blob = vm->input(0);
    blob_resize(blob, vm->input(1)->as_i());
    move(blob, vm->output());
}

void Blob__slice(VM* vm)
{
    Value* existingBlob = vm->input(0);
    int offset = vm->input(1)->as_i();
    int size = vm->input(2)->as_i();

    char* existingData;
    u32 existingNumBytes;
    blob_data(existingBlob, &existingData, &existingNumBytes);
    
    if ((offset + size) > existingNumBytes)
        return vm->throw_str("Offset+size is out of bounds");

    set_blob_slice(vm->output(), existingBlob, existingData+offset, size);
}

#define blob_set_macro(type, as) \
    Value* blob = vm->input(0); \
    int offset = vm->input(1)->as_i(); \
    \
    if ((offset + sizeof(type)) > blob_size(blob)) \
        return vm->throw_str("Offset is out of bounds"); \
    \
    char* data = blob_touch(blob); \
    *(type*)(data + offset) = as(vm->input(2)); \
    move(blob, vm->output());

void Blob__set_u8(VM* vm) { blob_set_macro(u8, as_int); }
void Blob__set_u16(VM* vm) { blob_set_macro(u16, as_int); }
void Blob__set_u32(VM* vm) { blob_set_macro(u32, as_int); }
void Blob__set_i8(VM* vm) { blob_set_macro(i8, as_int); }
void Blob__set_i16(VM* vm) { blob_set_macro(i16, as_int); }
void Blob__set_i32(VM* vm) { blob_set_macro(i32, as_int); }
void Blob__set_f32(VM* vm) { blob_set_macro(f32, as_float); }
void Blob__set_f64(VM* vm) { blob_set_macro(f64, as_float); }

#define blob_append_macro(type, as) \
    Value* blob = vm->input(0); \
    Value* val = vm->input(1); \
    char* data = blob_grow(blob, sizeof(type)); \
    *(type*)data = as(val); \
    move(blob, vm->output());

void Blob__append_u8(VM* vm) { blob_append_macro(u8, as_int); }
void Blob__append_u16(VM* vm) { blob_append_macro(u16, as_int); }
void Blob__append_u32(VM* vm) { blob_append_macro(u32, as_int); }
void Blob__append_i8(VM* vm) { blob_append_macro(i8, as_int); }
void Blob__append_i16(VM* vm) { blob_append_macro(i16, as_int); }
void Blob__append_i32(VM* vm) { blob_append_macro(i32, as_int); }
void Blob__append_f32(VM* vm) { blob_append_macro(f32, as_float); }
void Blob__append_f64(VM* vm) { blob_append_macro(f64, as_float); }

#define blob_get_macro(type, set) \
    Value* blob = vm->input(0); \
    int offset = vm->input(1)->as_i(); \
    \
    char* data; \
    u32 numBytes; \
    blob_data(blob, &data, &numBytes); \
    \
    if ((offset + sizeof(type)) > numBytes) \
        return vm->throw_str("Offset is out of bounds"); \
    \
    type result = *(type*) (data + offset); \
    set(vm->output(), result);

void Blob__u8(VM* vm) { blob_get_macro(u8, set_int); }
void Blob__u16(VM* vm) { blob_get_macro(u16, set_int); }
void Blob__u32(VM* vm) { blob_get_macro(u32, set_int); }
void Blob__i8(VM* vm) { blob_get_macro(i8, set_int); }
void Blob__i16(VM* vm) { blob_get_macro(i16, set_int); }
void Blob__i32(VM* vm) { blob_get_macro(i32, set_int); }
void Blob__f32(VM* vm) { blob_get_macro(f32, set_float); }
void Blob__f64(VM* vm) { blob_get_macro(f64, set_float); }

void blob_install_functions(NativePatch* patch)
{
    circa_patch_function(patch, "make_blob", make_blob);
    circa_patch_function(patch, "Blob.size", Blob__size);
    circa_patch_function(patch, "Blob.resize", Blob__resize);
    circa_patch_function(patch, "Blob.slice", Blob__slice);
    circa_patch_function(patch, "Blob.set_u8", Blob__set_u8);
    circa_patch_function(patch, "Blob.set_u16", Blob__set_u16);
    circa_patch_function(patch, "Blob.set_u32", Blob__set_u32);
    circa_patch_function(patch, "Blob.set_i8", Blob__set_i8);
    circa_patch_function(patch, "Blob.set_i16", Blob__set_i16);
    circa_patch_function(patch, "Blob.set_i32", Blob__set_i32);
    circa_patch_function(patch, "Blob.set_f32", Blob__set_f32);
    circa_patch_function(patch, "Blob.set_f64", Blob__set_f64);
    circa_patch_function(patch, "Blob.append_u8", Blob__append_u8);
    circa_patch_function(patch, "Blob.append_u16", Blob__append_u16);
    circa_patch_function(patch, "Blob.append_u32", Blob__append_u32);
    circa_patch_function(patch, "Blob.append_i8", Blob__append_i8);
    circa_patch_function(patch, "Blob.append_i16", Blob__append_i16);
    circa_patch_function(patch, "Blob.append_i32", Blob__append_i32);
    circa_patch_function(patch, "Blob.append_f32", Blob__append_f32);
    circa_patch_function(patch, "Blob.append_f64", Blob__append_f64);
    circa_patch_function(patch, "Blob.u8", Blob__u8);
    circa_patch_function(patch, "Blob.u16", Blob__u16);
    circa_patch_function(patch, "Blob.u32", Blob__u32);
    circa_patch_function(patch, "Blob.i8", Blob__i8);
    circa_patch_function(patch, "Blob.i16", Blob__i16);
    circa_patch_function(patch, "Blob.i32", Blob__i32);
    circa_patch_function(patch, "Blob.f32", Blob__f32);
    circa_patch_function(patch, "Blob.f64", Blob__f64);
}

} // namespace circa
