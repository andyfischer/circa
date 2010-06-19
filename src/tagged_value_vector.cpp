// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "builtin_types/list.h"
#include "debug_valid_objects.h"
#include "tagged_value.h"
#include "tagged_value_vector.h"

namespace circa {
namespace tagged_value_vector {

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

ListData* create_list(int capacity)
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

ListData* duplicate(ListData* source)
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

ListData* increase_capacity(ListData* original, int new_capacity)
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

ListData* grow_capacity(ListData* original)
{
    if (original == NULL)
        return create_list(1);

    ListData* result = increase_capacity(original, original->capacity * 2);
    return result;
}

ListData* resize(ListData* original, int numElements)
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
    ListData* result = touch(original);

    // Possibly set extra elements to null, if we are shrinking.
    for (int i=numElements; i < result->count; i++)
        make_null(&result->items[i]);
    result->count = numElements;

    return result;
}

void clear(ListData** data)
{
    if (*data == NULL) return;
    decref(*data);
    *data = NULL;
}

ListData* touch(ListData* data)
{
    if (data == NULL)
        return NULL;
    assert(data->refCount > 0);
    if (data->refCount == 1)
        return data;

    ListData* copy = duplicate(data);
    decref(data);
    return copy;
}

TaggedValue* append(ListData** data)
{
    if (*data == NULL) {
        *data = create_list(1);
    } else {
        *data = touch(*data);
        
        if ((*data)->count == (*data)->capacity)
            *data = grow_capacity(*data);
    }

    ListData* d = *data;
    d->count++;
    return &d->items[d->count - 1];
}

TaggedValue* get_index(ListData* data, int index)
{
    if (data == NULL)
        return NULL;
    if (index >= data->count)
        return NULL;
    return &data->items[index];
}

void set_index(ListData** data, int index, TaggedValue* v)
{
    *data = touch(*data);
    copy(v, get_index(*data, index));
}

int num_elements(ListData* data)
{
    if (data == NULL) return 0;
    return data->count;
}

void remove_and_replace_with_back(ListData** data, int index)
{
    *data = touch(*data);
    assert(index < (*data)->count);

    make_null(&(*data)->items[index]);

    int lastElement = (*data)->count - 1;
    if (index < lastElement)
        swap(&(*data)->items[index], &(*data)->items[lastElement]);

    (*data)->count--;
}

std::string to_string(ListData* value)
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

int refcount(ListData* value)
{
    if (value == NULL) return 0;
    return value->refCount;
}

}
}
