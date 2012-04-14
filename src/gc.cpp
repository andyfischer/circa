// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "gc.h"
#include "names.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

// Global GC zone
CircaObject* g_first = NULL;
bool g_currentlyCollecting = false;

void gc_register_object(CircaObject* obj)
{
    ca_assert(strcmp(obj->magicalHeader, "caobj") == 0);
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
        if (current->permanent)
            gc_mark(&toMark, current, color);
    }

    // Mark everything else with a breadth-first search.
    GCReferenceList currentlyMarking;
    while (toMark.count > 0) {

        gc_ref_list_swap(&toMark, &currentlyMarking);
        gc_ref_list_reset(&toMark);

        for (int i=0; i < currentlyMarking.count; i++) {
            CircaObject* object = currentlyMarking.refs[i];

            gc_mark(&toMark, object, color);
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

void gc_ref_list_reset(GCReferenceList* list)
{
    list->count = 0;
}

int gc_count_live_objects()
{
    int count = 0;
    for (CircaObject* current = g_first; current != NULL; current = current->next)
        count += 1;
    return count;
}

void gc_dump_live_objects()
{
    for (CircaObject* current = g_first; current != NULL; current = current->next) {
        std::cout << name_to_string(current->type->name)
            << "@" << current << std::endl;
    }
}

bool gc_sanity_check_live_objects()
{
    for (CircaObject* current = g_first; current != NULL; current = current->next) {
        if (strcmp(current->magicalHeader, "caobj") != 0) {
            std::cout << "sanity check failed on live object:" << std::endl;
            return false;
        }
    }
    return true;
}

void gc_mark(GCReferenceList* refList, CircaObject* object, GCColor color)
{
    if (object == NULL)
        return;

    if (strcmp(object->magicalHeader, "caobj") != 0)
        internal_error("called gc_mark on not a CircaObject");
    
    if (object->gcColor == color)
        return;

    object->gcColor = color;

    gc_mark(refList, (CircaObject*) object->type, color);

    if (object->type->gcListReferences != NULL)
        object->type->gcListReferences(object, refList, color);

    refList->count += 1;
    refList->refs = (CircaObject**) realloc(refList->refs, sizeof(CircaObject*) * refList->count);
    refList->refs[refList->count - 1] = object;
}

void gc_mark_tagged_value(GCReferenceList* list, caValue* value, GCColor color)
{
    if (value->value_type != NULL)
        gc_mark(list, (CircaObject*) value->value_type, color);

    // TODO: Follow the value as well
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

void gc_register_new_object(CircaObject* obj, Type* type, bool permanent)
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
    
    ca_assert(type != NULL);
}

void ogc_n_object_deleted(CircaObject* obj)
{
    memset(obj->magicalHeader, 0, 6);

    gc_on_object_deleted(obj);
}

void gc_set_object_permanent(CircaObject* obj, bool permanent)
{
    obj->permanent = permanent;
}

} // namespace circa
