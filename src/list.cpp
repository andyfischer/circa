// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <string>
#include <sstream>

#include "list.h"
#include "tagged_value.h"
#include "tagged_value_accessors.h"
#include "type.h"

namespace circa {

namespace list_t {

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
    result->refCount = 0;
    result->count = 0;
    result->capacity = capacity;
    incref(result);
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

    for (int i=0; i < source->count; i++)
        copy(&source->items[i], &result->items[i]);

    result->count = source->count;

    return result;
}

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
static ListData* grow_capacity(ListData* original)
{
    if (original == NULL)
        return create_list(1);

    int new_capacity = original->capacity * 2;
    ListData* result = create_list(new_capacity);
    result->capacity = new_capacity;
    result->count = original->count;
    for (int i=0; i < original->count; i++)
        swap(&original->items[i], &result->items[i]);
    decref(original);
    return result;
}

// Reset this list to have 0 elements.
static void clear(ListData** data)
{
    decref(*data);
    *data = NULL;
}

// Return a version of this list which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
ListData* begin_modify(ListData* data)
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
        *data = begin_modify(*data);
        
        if ((*data)->count == (*data)->capacity)
            *data = grow_capacity(*data);
    }

    ListData* d = *data;
    d->count++;
    return &d->items[d->count - 1];
}

TaggedValue* append(TaggedValue* list)
{
    return append((ListData**) &list->value_data);
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
    ListData* data = (ListData*) get_pointer(value);
    if (data == NULL) return;
    decref(data);
}

void tv_copy(TaggedValue* source, TaggedValue* dest)
{
    ListData* s = (ListData*) get_pointer(source);
    ListData* d = (ListData*) get_pointer(dest);

    if (s != NULL)
        incref(s);
    if (d != NULL)
        decref(d);
    
    set_pointer(dest, s);
}

std::string tv_to_string(TaggedValue* value)
{
    return to_string((ListData*) get_pointer(value));
}

void tv_begin_modify(TaggedValue* value)
{
    ListData* data = (ListData*) get_pointer(value);
    set_pointer(value, begin_modify(data));
}

void setup_type(Type* type)
{
    reset_type(type);
    type->initialize = tv_initialize;
    type->release = tv_release;
    type->copy = tv_copy;
    type->toString = tv_to_string;
    type->beginModify = tv_begin_modify;
}

void postponed_setup_type(Type*)
{
    // TODO: create member functions: append, count
}

} // namespace list_t

List::~List()
{
    if (_data != NULL)
        decref(_data);
}

List::List(List const& copy)
{
    _data = copy._data;
    if (_data != NULL)
        incref(_data);
}

List const&
List::operator=(List const& rhs)
{
    if (_data == rhs._data)
        return *this;
    if (_data != NULL)
        decref(_data);
    _data = rhs._data;
    if (_data != NULL)
        incref(_data);
    return *this;
}

int
List::length() const
{
    return _data == NULL ? 0 : _data->count;
}

TaggedValue*
List::append()
{
    return list_t::append(&_data);
}

void
List::clear()
{
    list_t::clear(&_data);
}

TaggedValue*
List::operator[](int index)
{
    assert(_data != NULL);
    assert(index < _data->count);
    return &_data->items[index];
}

}
