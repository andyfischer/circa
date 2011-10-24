// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "object.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

// Global GC zone
CircaObject* g_first = NULL;
bool g_currentlyCollecting = false;

void gc_register_object(CircaObject* obj)
{
    ca_assert(obj->next == NULL);
    ca_assert(obj->prev == NULL);

    // Can't create an object while collecting
    ca_assert(!g_currentlyCollecting);

    if (g_first == NULL) {
        g_first = obj;
    } else {
        obj->next = g_first;
        g_first->prev = obj;
        g_first = obj;
    }
}

void gc_on_object_deleted(CircaObject* obj)
{
    if (obj->next != NULL)
        obj->next->prev = obj->prev;
    if (obj->prev != NULL)
        obj->prev->next = obj->next;

    // Check if 'obj' is the first object
    if (obj == g_first)
        g_first = g_first->next;

    obj->next = NULL;
    obj->prev = NULL;
}

void gc_collect()
{
    // Alternate the color on each pass.
    char color = 1;
    static char s_lastColorUsed = 0;
    if (color == s_lastColorUsed)
        color = 2;
    s_lastColorUsed = color;
    g_currentlyCollecting = true;

    // First pass: find root objects, and accumulate their references.
    GCReferenceList toMark;
    for (CircaObject* current = g_first; current != NULL; current = current->next) {
        if (current->permanent) {
            current->gcColor = color;
            if (current->type->gcListReferences != NULL)
                current->type->gcListReferences(current, &toMark);
        }
    }

    // Mark everything else with a breadth-first search.
    GCReferenceList currentlyMarking;
    while (toMark.count > 0) {

        gc_ref_list_swap(&toMark, &currentlyMarking);
        gc_ref_list_reset(&toMark);

        for (int i=0; i < currentlyMarking.count; i++) {
            CircaObject* object = currentlyMarking.refs[i];

            // Skip objs that are already marked
            if (object->gcColor == color)
                continue;

            // Mark and fetch references, we'll search them on the next iteration.
            object->gcColor = color;
            if (object->type->gcListReferences != NULL)
                object->type->gcListReferences(object, &toMark);
        }
    }

    // Last pass: delete unmarked objects.
    for (CircaObject* current = g_first; current != NULL; ) {

        // Save 'next' pointer, in case the object is destroyed
        CircaObject* next = current->next;

        if (current->gcColor != color) {

            // Remove from linked list. This will nullify 'next' and 'prev'.
            gc_on_object_deleted(current);

            // Release object
            if (current->type->gcRelease != NULL)
                current->type->gcRelease(current);
        }

        current = next;
    }
    
    g_currentlyCollecting = false;
}

void gc_ref_append(GCReferenceList* list, CircaObject* item)
{
    if (item == NULL)
        return;

    list->count += 1;
    list->refs = (CircaObject**) realloc(list->refs, sizeof(CircaObject*) * list->count);
    list->refs[list->count - 1] = item;
}

void gc_ref_list_reset(GCReferenceList* list)
{
    list->count = 0;
}

void gc_ref_list_swap(GCReferenceList* a, GCReferenceList* b)
{
    int tempCount = a->count;
    CircaObject** tempRefs = a->refs;
    a->count = b->count;
    a->refs = b->refs;
    b->count = tempCount;
    b->refs = tempRefs;
}

} // namespace circa
