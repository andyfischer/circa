// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct GCHeader
{
    Type *type;

    // Linked list of all GC objects in this zone
    GCHeader* next;

    // Whether this object is a 'root', meaning that it cannot be collected.
    bool root;

    // Used during collection
    char color;

    GCHeader() : type(NULL), root(false) {}
};

struct GCReferenceList
{
    int count;
    GCHeader** refs;

    GCReferenceList() : count(0), refs(NULL) {}
    ~GCReferenceList() { free(refs); }
};

void gc_register_new_object(GCHeader* obj);
void gc_collect();

void gc_ref_append(GCReferenceList* list, GCHeader* item);
void gc_ref_list_reset(GCReferenceList* list);
void gc_ref_list_swap(GCReferenceList* a, GCReferenceList* b);

} // namespace circa
