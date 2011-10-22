// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

// Structure used during GC traversal
struct GCReferenceList
{
    int count;
    CircaObject** refs;

    GCReferenceList() : count(0), refs(NULL) {}
    ~GCReferenceList() { free(refs); }
};

// Remove object from the global object list. If the object owner wants to delete
// this object outside of garbage collection, they should call this.
void gc_on_object_deleted(CircaObject* obj);

void gc_collect();

// Add object to the global list. Should call this on object creation.
void gc_register_object(CircaObject* object);

void gc_ref_append(GCReferenceList* list, CircaObject* item);
void gc_ref_list_reset(GCReferenceList* list);

// Swap the contents of 'a' with 'b'
void gc_ref_list_swap(GCReferenceList* a, GCReferenceList* b);

} // namespace circa
