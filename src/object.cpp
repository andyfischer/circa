// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "object.h"
#include "list.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

bool is_object(caValue* value)
{
    if (value->value_type->storageType != name_StorageTypeObject)
        return false;

    // Sanity check, look at object's magical header.
#if DEBUG
    CircaObject* object = (CircaObject*) value->value_data.ptr;
    ca_assert(strcmp(object->magicalHeader, "caobj") == 0);
#endif

    return true;
}

CircaObject* as_object(caValue* value)
{
    ca_assert(is_object(value));
    return (CircaObject*) value->value_data.ptr;
}

CircaObject* alloc_object(Type* type, size_t objectSize)
{
    CircaObject* obj = (CircaObject*) malloc(sizeof(CircaObject) + objectSize);

    strcpy(obj->magicalHeader, "caobj");
    obj->refcount = 1;
    obj->type = type;
    obj->next = NULL;
    obj->prev = NULL;
    obj->root = true;
    obj->referenced = false;
    obj->gcColor = 0;
    
    return obj;
}

void object_initialize(Type* type, caValue* value)
{
    value->value_data.ptr = alloc_object(type, type->objectSize);
}

void object_release(caValue* value)
{
    CircaObject* object = (CircaObject*) value->value_data.ptr;
    ca_assert(is_object(value));

    ca_assert(object->refcount > 0);

    object->refcount--;

    if (object->refcount == 0) {

        Type* type = value->value_type;

        caObjectReleaseFunc func =
            (caObjectReleaseFunc) as_opaque_pointer(list_get(&type->parameter, 0));

        if (func != NULL)
            func(object_get_body(value));

        memset(object->magicalHeader, 0, 6);
        free(object);
    }
}

void object_copy(Type* type, caValue* source, caValue* dest)
{
    set_null(dest);

    ca_assert(is_object(source));
    CircaObject* object = (CircaObject*) source->value_data.ptr;
    ca_assert(object->refcount > 0);
    object->refcount++;
    dest->value_data.ptr = object;
    dest->value_type = source->value_type;
}

void* object_get_body(caValue* value)
{
    ca_assert(is_object(value));
    CircaObject* object = as_object(value);
    return (void*) object->body;
}

int object_hash(caValue* value)
{
    // Objects are only equal by identity.
    return value->value_data.asint;
}

void setup_object_type(Type* type, int objectSize, caObjectReleaseFunc releaseFunc)
{
    type->storageType = name_StorageTypeObject;
    type->initialize = object_initialize;
    type->copy = object_copy;
    type->release = object_release;
    type->objectSize = objectSize;
    type->hashFunc = object_hash;

    set_list(&type->parameter, 1);
    set_opaque_pointer(list_get(&type->parameter, 0), (void*) releaseFunc);
}

} // namespace circa

extern "C" {

using namespace circa;

bool circa_is_object(caValue* value)
{
    return is_object(value);
}
void* circa_object_contents(caValue* value)
{
    return object_get_body(value);
}

void* circa_object_input(caStack* stack, int input)
{
    return circa_object_contents(circa_input(stack, input));
}
void* circa_create_object_output(caStack* stack, int output)
{
    return circa_object_contents(circa_create_default_output(stack, output));
}
void circa_setup_object_type(caType* type, size_t objectSize, caObjectReleaseFunc release)
{
    ca_assert(!type->inUse);
    setup_object_type(type, objectSize, release);
}

} // extern "C"
