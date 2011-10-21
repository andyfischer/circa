// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

// Global GC zone
GCHeader* g_firstObject = NULL;
GCHeader* g_lastObject = NULL;
char g_lastColorUsed = 0;

void gc_register_new_object(GCHeader* obj)
{
    obj->next = NULL;
    obj->color = 0;

    if (g_firstObject == NULL) {
        g_firstObject = obj;
        g_lastObject = obj;
    } else {
        ca_assert(g_lastObject->next == NULL);
        g_lastObject->next = obj;
        g_lastObject = obj;
    }
}

void gc_collect()
{
    char color = 1;
    if (color == g_lastColorUsed)
        color = 2;
    g_lastColorUsed = color;

    GCReferenceList toMark;

    // First pass: mark roots
    for (GCHeader* current = g_firstObject; current != NULL; current = current->next) {
        if (current->root) {
            current->color = color;
            current->type->gcListReferences(current, &toMark);
        }
    }

    GCReferenceList currentlyMarking;

    // Breadth first search to mark remaining things
    while (toMark.count > 0) {

        gc_ref_list_swap(&toMark, &currentlyMarking);
        gc_ref_list_reset(&toMark);

        for (int i=0; i < currentlyMarking.count; i++) {
            GCHeader* object = currentlyMarking.refs[i];

            // Skip objs that are already marked
            if (object->color == color)
                continue;

            object->color = color;
            object->type->gcListReferences(object, &toMark);
        }
    }

    // Last pass: delete unmarked objects
    GCHeader* previous = NULL;
    for (GCHeader* current = g_firstObject; current != NULL; ) {

        // Save ->next pointer, in case object is destroyed
        GCHeader* next = current->next;

        if (current->color != color) {

            // Remove from linked list
            if (current == g_firstObject) {
                g_firstObject = current->next;
            } else {
                previous->next = current->next;
            }

            if (current == g_lastObject) {
                g_lastObject = previous;
                if (g_lastObject != NULL)
                    g_lastObject->next = NULL;
            }

            // Release object
            if (current->type->gcRelease != NULL)
                current->type->gcRelease(current);
        } else {
            previous = current;
        }

        current = next;
    }
}

void gc_ref_append(GCReferenceList* list, GCHeader* item)
{
    if (item == NULL)
        return;

    list->count += 1;
    list->refs = (GCHeader**) realloc(list->refs, sizeof(GCHeader*) * list->count);
    list->refs[list->count - 1] = item;
}

void gc_ref_list_reset(GCReferenceList* list)
{
    list->count = 0;
}

void gc_ref_list_swap(GCReferenceList* a, GCReferenceList* b)
{
    int tempCount = a->count;
    GCHeader** tempRefs = a->refs;
    a->count = b->count;
    a->refs = b->refs;
    b->count = tempCount;
    b->refs = tempRefs;
}

} // namespace circa
