// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "list_t.h"
#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "type.h"

namespace circa {
namespace list_t {
    
bool is_list(TaggedValue* value);

struct ListData {
    int refCount;
    int count;
    int capacity;
    TaggedValue items[0];
};

void incref(ListData* data)
{
    data->refCount++;
}

void decref(ListData* data)
{
    assert(data->refCount > 0);
    data->refCount--;
    if (data->refCount == 0) {
        // Release all elements
        for (int i=0; i < data->count; i++)
            make_null(&data->items[i]);
        free(data);
    }
}

// Create a new list, starts off with 1 ref.
static ListData* create_list(int capacity)
{
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
    result->refCount = 1;
    result->count = 0;
    result->capacity = capacity;
    for (int i=0; i < capacity; i++)
        result->items[i].init();
    return result;
}

// Creates a new list that is a duplicate of source. Starts off with 1 ref.
// Does not decref source.
static ListData* duplicate(ListData* source)
{
    if (source == NULL || source->count == 0)
        return NULL;

    ListData* result = create_list(source->count);
    result->count = source->count;

    for (int i=0; i < source->count; i++)
        copy(&source->items[i], &result->items[i]);

    return result;
}

// Returns a possibly new list with the given capacity.
static ListData* resize(ListData* original, int new_capacity)
{
    if (original == NULL)
        return create_list(new_capacity);

    if (original->capacity == new_capacity)
        return original;

    ListData* result = create_list(new_capacity);
    result->count = new_capacity < original->count ? new_capacity : original->count;
    for (int i=0; i < result->count; i++)
        swap(&original->items[i], &result->items[i]);
    decref(original);
    return result;
}

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
static ListData* grow_capacity(ListData* original)
{
    if (original == NULL)
        return create_list(1);

    return resize(original, original->capacity * 2);
}

// Reset this list to have 0 elements.
static void clear(ListData** data)
{
    if (*data == NULL) return;
    decref(*data);
    *data = NULL;
}

// Return a version of this list which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
ListData* mutate(ListData* data)
{
    assert(data->refCount > 0);
    if (data->refCount == 1)
        return data;

    ListData* copy = duplicate(data);
    decref(data);
    return copy;
}

// Add a new blank element to the end of the list, resizing if necessary.
// Returns the new element.
TaggedValue* append(ListData** data)
{
    if (*data == NULL) {
        *data = create_list(1);
    } else {
        *data = mutate(*data);
        
        if ((*data)->count == (*data)->capacity)
            *data = grow_capacity(*data);
    }

    ListData* d = *data;
    d->count++;
    return &d->items[d->count - 1];
}

TaggedValue* append(TaggedValue* list)
{
    assert(is_list(list));
    return append((ListData**) &list->value_data);
}

void resize(TaggedValue* list, int newSize)
{
    assert(is_list(list));
    set_pointer(list, resize((ListData*) get_pointer(list), newSize));
}

void clear(TaggedValue* list)
{
    assert(is_list(list));
    clear((ListData**) &list->value_data);
}

static std::string to_string(ListData* value)
{
    if (value == NULL)
        return "[]";

    std::stringstream out;
    out << "[";
    for (int i=0; i < value->count; i++) {
        if (i > 0) out << ", ";
        out << to_string(&value->items[i]);
    }
    out << "]";
    return out.str();
}

void tv_initialize(Type* type, TaggedValue* value)
{
    set_pointer(value, NULL);
}

void tv_release(TaggedValue* value)
{
    assert(is_list(value));
    ListData* data = (ListData*) get_pointer(value);
    if (data == NULL) return;
    decref(data);
}

void tv_copy(TaggedValue* source, TaggedValue* dest)
{
    assert(is_list(source));
    assert(is_list(dest));
    ListData* s = (ListData*) get_pointer(source);
    ListData* d = (ListData*) get_pointer(dest);

    if (s != NULL)
        incref(s);
    if (d != NULL)
        decref(d);
    
    set_pointer(dest, s);
}

TaggedValue* tv_get_element(TaggedValue* value, int index)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL) return NULL;
    if (index >= s->count) return NULL;
    return &s->items[index];
}

int tv_num_elements(TaggedValue* value)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL) return 0;
    return s->count;
}

std::string tv_to_string(TaggedValue* value)
{
    assert(is_list(value));
    return to_string((ListData*) get_pointer(value));
}

void tv_mutate(TaggedValue* value)
{
    assert(is_list(value));
    ListData* data = (ListData*) get_pointer(value);
    set_pointer(value, mutate(data));
}

bool is_list(TaggedValue* value)
{
    return value->value_type->initialize == tv_initialize;
}

void setup_type(Type* type)
{
    reset_type(type);
    type->initialize = tv_initialize;
    type->release = tv_release;
    type->copy = tv_copy;
    type->toString = tv_to_string;
    type->getElement = tv_get_element;
    type->numElements = tv_num_elements;
    type->mutate = tv_mutate;
}

void postponed_setup_type(Type*)
{
    // TODO: create member functions: append, count
}

int get_refcount(ListData* data)
{
    return data->refCount;
}

} // namespace list_t

List::List()
  : TaggedValue()
{
    change_type(this, LIST_T);
}

TaggedValue*
List::append()
{
    return list_t::append((TaggedValue*) this);
}

void
List::clear()
{
    list_t::clear((TaggedValue*) this);
}

int
List::length()
{
    return list_t::tv_num_elements((TaggedValue*) this);
}

TaggedValue*
List::operator[](int index)
{
    return list_t::tv_get_element((TaggedValue*) this, index);
}

}
