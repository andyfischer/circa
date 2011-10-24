// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "gc.h"
#include "object.h"

namespace circa {

void register_new_object(CircaObject* obj, Type* type, bool permanent)
{
    CircaObject* header = (CircaObject*) obj;

    strcpy(header->magicalHeader, "caobj");

    ca_assert(type != NULL);

    obj->type = type;
    obj->next = NULL;
    obj->prev = NULL;
    obj->permanent = permanent;
    obj->gcColor = 0;

    gc_register_object(header);
}

void on_object_deleted(CircaObject* obj)
{
    memset(obj->magicalHeader, 0, 6);

    gc_on_object_deleted(obj);
}

void set_object_permanent(CircaObject* obj, bool permanent)
{
    obj->permanent = permanent;
}

} // namespace circa
