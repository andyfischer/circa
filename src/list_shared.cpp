// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "debug_valid_objects.h"
#include "list_shared.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

ListData* allocate_empty_list(int capacity)
{
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(TaggedValue));
    debug_register_valid_object(result, LIST_OBJECT);

    result->refCount = 1;
    result->count = 0;
    result->capacity = capacity;
    memset(result->items, 0, capacity * sizeof(TaggedValue));
    for (int i=0; i < capacity; i++)
        result->items[i].init();

    //std::cout << "created list " << result << std::endl;

    return result;
}

ListData* allocate_list(int size)
{
    ListData* result = allocate_empty_list(size);
    result->count = size;
    return result;
}

void free_list(ListData* data)
{
    // Release all elements
    for (int i=0; i < data->count; i++)
        set_null(&data->items[i]);
    free(data);
    debug_unregister_valid_object(data);
}

TaggedValue* list_get_element(TaggedValue* value, int index)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_LIST);

    ListData* data = (ListData*) value->value_data.ptr;
    ca_assert(data->count >= index);

    return &data->items[index];
}

void list_remove_element(TaggedValue* list, int index)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);

    ListData* data = (ListData*) list->value_data.ptr;

    for (int i=index; i < (data->count - 1); i++)
        swap(&data->items[i], &data->items[i + 1]);

    set_null(&data->items[data->count - 1]);
    data->count--;
}

} // namespace circa
