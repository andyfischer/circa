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
    if (data->refCount == 0)
        free(data);
}

// Create a new list, starts off with 1 reference.
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

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
static ListData* grow(ListData* original)
{
    int new_capacity = original->capacity * 2;
    ListData* result = create_list(new_capacity);
    result->capacity = new_capacity;
    result->count = original->count;
    for (int i=0; i < original->count; i++)
        swap(&original->items[i], &result->items[i]);
    decref(original);
    return result;
}

static std::string to_string(ListData* value)
{
    if (value == NULL)
        return "[]";

    std::stringstream out;
    out << "[";
    for (int i=0; i < value->count; i++) {
        if (i > 0) out << ",";
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

std::string tv_to_string(TaggedValue* value)
{
    return to_string((ListData*) get_pointer(value));
}

void setup_type(Type* type)
{
    reset_type(type);
    type->initialize = tv_initialize;
    type->release = tv_release;
    type->toString = tv_to_string;
}

} // namespace list_t

List::~List()
{
    if (_data != NULL)
        decref(_data);
}

int
List::length() const
{
    return _data == NULL ? 0 : _data->count;
}

TaggedValue*
List::append()
{
    if (_data == NULL) {
        _data = list_t::create_list(1);
    } else if (_data->count == _data->capacity) {
        _data = list_t::grow(_data);
    }

    _data->count++;
    return &_data->items[_data->count - 1];
}

void
List::clear()
{
    list_t::decref(_data);
    _data = NULL;
}

TaggedValue*
List::operator[](int index)
{
    assert(_data != NULL);
    assert(index < _data->count);
    return &_data->items[index];
}

}
