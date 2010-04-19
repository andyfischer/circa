// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "debug_valid_objects.h"
#include "list_t.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {
namespace list_t {

bool is_list(TaggedValue* value);
void tv_mutate(TaggedValue* value);
static ListData* mutate(ListData* data);

struct ListData {
    int refCount;
    int count;
    int capacity;
    TaggedValue items[0];
};

void assert_valid_list(ListData* list)
{
    if (list == NULL) return;
    assert(list->refCount > 0);
    debug_assert_valid_object(list, LIST_OBJECT);
}

void incref(ListData* data)
{
    assert_valid_list(data);
    data->refCount++;
}

void decref(ListData* data)
{
    assert_valid_list(data);
    assert(data->refCount > 0);
    data->refCount--;

    if (data->refCount == 0) {
        // Release all elements
        for (int i=0; i < data->count; i++)
            make_null(&data->items[i]);
        free(data);
        debug_unregister_valid_object(data);
    }
}

// Create a new list, starts off with 1 ref.
static ListData* create_list(int capacity)
{
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
    debug_register_valid_object(result, LIST_OBJECT);

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

    assert_valid_list(source);

    ListData* result = create_list(source->capacity);

    result->count = source->count;

    for (int i=0; i < source->count; i++)
        copy(&source->items[i], &result->items[i]);

    return result;
}

// Returns a new list with the given capacity.
static ListData* increase_capacity(ListData* original, int new_capacity)
{
    if (original == NULL)
        return create_list(new_capacity);

    assert_valid_list(original);
    ListData* result = create_list(new_capacity);

    result->count = original->count;
    for (int i=0; i < result->count; i++)
        copy(&original->items[i], &result->items[i]);

    decref(original);
    return result;
}

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
static ListData* grow_capacity(ListData* original)
{
    if (original == NULL)
        return create_list(1);

    ListData* result = increase_capacity(original, original->capacity * 2);
    return result;
}

// Modify a list so that it has the given number of elements.
static ListData* resize(ListData* original, int numElements)
{
    if (original == NULL) {
        if (numElements == 0)
            return NULL;
        ListData* result = create_list(numElements);
        result->count = numElements;
        return result;
    }

    if (numElements == 0) {
        decref(original);
        return NULL;
    }

    // Check for not enough capacity
    if (numElements > original->capacity) {
        ListData* result = increase_capacity(original, numElements);
        result->count = numElements;
        return result;
    }

    if (original->count == numElements)
        return original;

    // Capacity is good, will need to modify 'count' on list and possibly
    // set some items to null. This counts as a modification.
    ListData* result = mutate(original);

    // Possibly set extra elements to null, if we are shrinking.
    for (int i=numElements; i < result->count; i++)
        make_null(&result->items[i]);
    result->count = numElements;

    return result;
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
static ListData* mutate(ListData* data)
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
    assert_valid_list(data);
    if (data == NULL) return;
    decref(data);
}

void tv_copy(TaggedValue* source, TaggedValue* dest)
{
    assert(is_list(source));
    assert(is_list(dest));
    ListData* s = (ListData*) get_pointer(source);
    ListData* d = (ListData*) get_pointer(dest);

    assert_valid_list(s);
    assert_valid_list(d);

    set_pointer(dest, duplicate(s));

    if (s != NULL)
        incref(s);
    if (d != NULL)
        decref(d);
    
    set_pointer(dest, s);
}

TaggedValue* tv_get_index(TaggedValue* value, int index)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL) return NULL;
    if (index >= s->count) return NULL;
    return &s->items[index];
}

void tv_set_index(TaggedValue* value, int index, TaggedValue* element)
{
    assert(is_list(value));
    ListData* s = (ListData*) get_pointer(value);
    assert(s);
    assert(s->count > index);

    tv_mutate(value);
    copy(element, &s->items[index]);
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

bool tv_matches_type(Type* type, Term* term)
{
    return declared_type(term)->initialize == tv_initialize;
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
    type->getIndex = tv_get_index;
    type->setIndex = tv_set_index;
    type->numElements = tv_num_elements;
    type->mutate = tv_mutate;
    type->matchesType = tv_matches_type;
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
List::get(int index)
{
    return list_t::tv_get_index((TaggedValue*) this, index);
}

void
List::resize(int newSize)
{
    list_t::resize(this, newSize); 
}

}
