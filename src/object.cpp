// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "object.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

bool is_object(caValue* value)
{
    return value->value_type->storageType == STORAGE_TYPE_OBJECT;
}

CircaObject* alloc_object(Type* type, size_t objectSize)
{
    CircaObject* obj = (CircaObject*) malloc(sizeof(CircaObject) + objectSize);

    strcpy(obj->magicalHeader, "caobj");
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

    ca_assert(object->refcount > 0);

    object->refcount--;

    if (object->refcount == 0) {
        memset(object->magicalHeader, 0, 6);
        free(object);
    }
}

void object_copy(Type* type, caValue* source, caValue* dest)
{
    set_null(dest);

    CircaObject* object = (CircaObject*) source->value_data.ptr;
    ca_assert(object->refcount > 0);
    object->refcount++;
    dest->value_data.ptr = object;
    dest->value_type = source->value_type;
}

void setup_object_type(Type* type, int objectSize)
{
    type->storageType = STORAGE_TYPE_OBJECT;
    type->initialize = object_initialize;
    type->copy = object_copy;
    type->release = object_release;
    type->objectSize = objectSize;
}

} // namespace circa
