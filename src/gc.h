// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct ObjectHeader
{
    char debugLabel[4];
    ObjectHeader(const char* label)
    {
        memcpy(&debugLabel, label, 4);
    };
};

struct ObjectListElement
{
    ObjectListElement* next;
    ObjectListElement* prev;
    ObjectHeader* obj;
};

struct ObjectList
{
    ObjectListElement* first;
    ObjectListElement* last;
};

void recursive_dump_heap(TaggedValue* value, const char* prefix);
int count_references_to_pointer(TaggedValue* container, void* ptr);
void list_references_to_pointer(TaggedValue* container, void* ptr,
        TaggedValue* outputList);

void append_to_object_list(ObjectList* list, ObjectListElement* element,
        ObjectHeader* header);
void remove_from_object_list(ObjectList* list, ObjectListElement* element);

} // namespace circa
