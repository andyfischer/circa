// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct GcHeader
{
    GcHeader* next;
    GcHeader* prev;
};

struct GcHeap
{
    GcHeader* firstObject;
    GcHeader* lastObject;
};

void recursive_dump_heap(TaggedValue* value, const char* prefix);
int count_references_to_pointer(TaggedValue* container, void* ptr);
void list_references_to_pointer(TaggedValue* container, void* ptr,
        TaggedValue* outputList);

void initialize_new_gc_object(GcHeap* heap, GcHeader* header);
void remove_gc_object(GcHeap* heap, GcHeader* header);

} // namespace circa
