// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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
TaggedValue* list_append(ListData** dataPtr)
{
    if (*dataPtr == NULL) {
        *dataPtr = allocate_empty_list(1);
    } else {
        *dataPtr = list_touch(*dataPtr);
        
        if ((*dataPtr)->count == (*dataPtr)->capacity)
            *dataPtr = list_double_capacity(*dataPtr);
    }

    ListData* data = *dataPtr;
    data->count++;
    return &data->items[data->count - 1];
}

TaggedValue* list_insert(ListData** dataPtr, int index)
{
    list_append(dataPtr);

    ListData* data = *dataPtr;

    // Move everything over, up till 'index'.
    for (int i = data->count - 1; i >= (index + 1); i--)
        swap(&data->items[i], &data->items[i - 1]);

    return &data->items[index];
}

TaggedValue* list_get_index(ListData* data, int index)
{
    if (data == NULL)
        return NULL;
    if (index >= data->count)
        return NULL;
    return &data->items[index];
}
void list_set_index(ListData* data, int index, TaggedValue* value)
{
    TaggedValue* dest = list_get_index(data, index);
    ca_assert(dest != NULL);
    copy(value, dest);
}

void list_remove_and_replace_with_last_element(ListData** data, int index)
{
    *data = list_touch(*data);
    ca_assert(index < (*data)->count);

    set_null(&(*data)->items[index]);

    int lastElement = (*data)->count - 1;
    if (index < lastElement)
        swap(&(*data)->items[index], &(*data)->items[lastElement]);

    (*data)->count--;
}

void list_remove_nulls(ListData** dataPtr)
{
    if (*dataPtr == NULL)
        return;

    *dataPtr = list_touch(*dataPtr);
    ListData* data = *dataPtr;

    int numRemoved = 0;
    for (int i=0; i < data->count; i++) {
        if (is_null(&data->items[i]))
            numRemoved++;
        else
            swap(&data->items[i - numRemoved], &data->items[i]);
    }
    *dataPtr = list_resize(*dataPtr, data->count - numRemoved);
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
    return list_get_index((ListData*) value->value_data.ptr, index);
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

TaggedValue* list_append(TaggedValue* list)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    TaggedValue* result = list_append(&data);
    list->value_data.ptr = data;
    return result;
}
TaggedValue* list_insert(TaggedValue* list, int index)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    TaggedValue* result = list_insert(&data, index);
    list->value_data.ptr = data;
    return result;
}

void list_remove_and_replace_with_last_element(TaggedValue* value, int index)
{
    ca_assert(is_list(value));
    list_remove_and_replace_with_last_element((ListData**) &value->value_data, index);
}

void list_remove_nulls(TaggedValue* value)
{
    ca_assert(is_list(value));
    list_remove_nulls((ListData**) &value->value_data);
}

ListType list_get_parameter_type(TaggedValue* parameter)
{
    if (is_null(parameter))
        return LIST_UNTYPED;
    if (is_type(parameter))
        return LIST_TYPED_UNSIZED;

    if (is_list(parameter)) {
        if ((list_get_length(parameter) == 2) && is_list(list_get_index(parameter, 0)))
            return LIST_TYPED_SIZED_NAMED;
        else
            return LIST_TYPED_SIZED;
    }
    return LIST_INVALID_PARAMETER;
}

bool list_type_has_specific_size(TaggedValue* parameter)
{
    return is_list(parameter);
}

void list_initialize_parameter_from_type_decl(Branch* typeDecl, TaggedValue* parameter)
{
    List& param = *set_list(parameter, 2);
    List& types = *set_list(param[0], typeDecl->length());
    List& names = *set_list(param[1], typeDecl->length());

    for (int i=0; i < typeDecl->length(); i++) {
        set_type(types[i], declared_type(typeDecl->get(i)));
        set_string(names[i], typeDecl->get(i)->name);
    }
}

TaggedValue* list_get_type_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    TaggedValue* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case LIST_TYPED_SIZED:
        return parameter;
    case LIST_TYPED_SIZED_NAMED:
        return list_get_index(parameter, 0);
    case LIST_UNTYPED:
    case LIST_TYPED_UNSIZED:
    case LIST_INVALID_PARAMETER:
        return NULL;
    }
    ca_assert(false);
    return NULL;
}

TaggedValue* list_get_name_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    TaggedValue* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case LIST_TYPED_SIZED_NAMED:
        return list_get_index(parameter, 1);
    case LIST_TYPED_SIZED:
    case LIST_UNTYPED:
    case LIST_TYPED_UNSIZED:
    case LIST_INVALID_PARAMETER:
        return NULL;
    }
    ca_assert(false);
    return NULL;
}
Type* list_get_repeated_type_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    return as_type(&type->parameter);
}
int list_find_field_index_by_name(Type* listType, std::string const& name)
{
    TaggedValue* nameList = list_get_name_list_from_type(listType);
    if (nameList == NULL)
        return -1;

    List& names = *as_list(nameList);
    for (int i=0; i < names.length(); i++)
        if (as_string(names[i]) == name)
            return i;
    return -1;
}

} // namespace circa
