// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "errors.h"
#include "heap_debugging.h"
#include "list_shared.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

void assert_valid_list(ListData* list)
{
    if (list == NULL) return;
    debug_assert_valid_object(list, LIST_OBJECT);
    if (list->refCount == 0) {
        std::stringstream err;
        err << "list has zero refs: " << list;
        internal_error(err.str().c_str());
    }
    ca_assert(list->refCount > 0);
}

ListData* allocate_empty_list(int capacity)
{
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
    debug_register_valid_object(result, LIST_OBJECT);

    result->refCount = 1;
    result->count = 0;
    result->capacity = capacity;
    memset(result->items, 0, capacity * sizeof(TaggedValue));
    for (int i=0; i < capacity; i++)
        result->items[i].initializeNull();

    //std::cout << "created list " << result << std::endl;

    return result;
}

ListData* allocate_list(int size)
{
    ListData* result = allocate_empty_list(size);
    result->count = size;
    return result;
}

void list_decref(ListData* data)
{
    assert_valid_list(data);
    ca_assert(data->refCount > 0);
    data->refCount--;

    if (data->refCount == 0)
        free_list(data);

    //std::cout << "decref " << data << " to " << data->refCount << std::endl;
}

void list_incref(ListData* data)
{
    assert_valid_list(data);
    data->refCount++;

    //std::cout << "incref " << data << " to " << data->refCount << std::endl;
}

void free_list(ListData* data)
{
    // Release all elements
    for (int i=0; i < data->count; i++)
        set_null(&data->items[i]);
    free(data);
    debug_unregister_valid_object(data, LIST_OBJECT);
}

ListData* list_touch(ListData* original)
{
    if (original == NULL)
        return NULL;
    ca_assert(original->refCount > 0);
    if (original->refCount == 1)
        return original;

    ListData* copy = list_duplicate(original);
    list_decref(original);
    return copy;
}

ListData* list_duplicate(ListData* source)
{
    if (source == NULL || source->count == 0)
        return NULL;

    assert_valid_list(source);

    ListData* result = allocate_empty_list(source->capacity);

    result->count = source->count;

    for (int i=0; i < source->count; i++)
        copy(&source->items[i], &result->items[i]);

    return result;
}

ListData* list_increase_capacity(ListData* original, int new_capacity)
{
    if (original == NULL)
        return allocate_empty_list(new_capacity);

    assert_valid_list(original);
    ListData* result = allocate_empty_list(new_capacity);

    bool createCopy = original->refCount > 1;

    result->count = original->count;
    for (int i=0; i < result->count; i++) {
        TaggedValue* left = &original->items[i];
        TaggedValue* right = &result->items[i];
        if (createCopy)
            copy(left, right);
        else
            swap(left, right);
    }

    list_decref(original);
    return result;
}

ListData* list_double_capacity(ListData* original)
{
    if (original == NULL)
        return allocate_empty_list(1);

    ListData* result = list_increase_capacity(original, original->capacity * 2);
    return result;
}

ListData* list_resize(ListData* original, int numElements)
{
    if (original == NULL) {
        if (numElements == 0)
            return NULL;
        ListData* result = allocate_empty_list(numElements);
        result->count = numElements;
        return result;
    }

    if (numElements == 0) {
        list_decref(original);
        return NULL;
    }

    // Check for not enough capacity
    if (numElements > original->capacity) {
        ListData* result = list_increase_capacity(original, numElements);
        result->count = numElements;
        return result;
    }

    if (original->count == numElements)
        return original;

    // Capacity is good, will need to modify 'count' on list and possibly
    // set some items to null. This counts as a modification.
    ListData* result = list_touch(original);

    // Possibly set extra elements to null, if we are shrinking.
    for (int i=numElements; i < result->count; i++)
        set_null(&result->items[i]);
    result->count = numElements;

    return result;
}

int list_get_length(TaggedValue* value)
{
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL)
        return 0;
    return s->count;
}

TaggedValue* list_get_index(TaggedValue* value, int index)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_LIST);

    ListData* data = (ListData*) value->value_data.ptr;
    ca_assert(index < data->count);

    return &data->items[index];
}
TaggedValue* list_get_index_from_end(TaggedValue* value, int reverseIndex)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_LIST);

    ListData* data = (ListData*) value->value_data.ptr;

    int index = data->count - reverseIndex - 1;
    ca_assert(index >= 0);
    ca_assert(index < data->count);

    return &data->items[index];
}

ListData* list_remove_index(ListData* original, int index)
{
    ca_assert(index < original->count);
    ListData* result = list_touch(original);

    for (int i=index; i < result->count - 1; i++)
        swap(&result->items[i], &result->items[i+1]);
    set_null(&result->items[result->count - 1]);
    result->count--;
    return result;
}

void list_remove_index(TaggedValue* list, int index)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);

    ListData* data = (ListData*) list->value_data.ptr;
    list_remove_index(data, index);
    list->value_data.ptr = data;
}

} // namespace circa
