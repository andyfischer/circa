// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Header used for any objects that participate in GC
struct CircaObject
{
    char magicalHeader[6];

    Type* type;

    // Nearby GCable objects.
    CircaObject* next;
    CircaObject* prev;

    // If we're 'referenced', then we can only be deleted by a GC pass, not manually.
    bool referenced;

    // If we're 'root', then we can only be deleted manually, not by GC.
    bool root;

    int refcount;

    // Used during GC collection
    GCColor gcColor;
};

bool is_object(caValue* value);

void setup_object_type(Type* type, int objectSize);

} // namespace circa
