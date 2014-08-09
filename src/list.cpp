// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "function.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

void ListData::dump()
{
    Value str;
    list_to_string(this, &str);
    printf("%s\n", as_cstring(&str));
}

size_t list_size(int capacity)
{
    return sizeof(ListData) + capacity * sizeof(Value);
}

ListData* allocate_list(int count, int capacity)
{
    ca_assert(count <= capacity);

    ListData* result = (ListData*) malloc(list_size(capacity));

    result->refCount = 1;
    result->count = count;
    result->capacity = capacity;
    for (int i=0; i < capacity; i++)
        initialize_null(&result->items[i]);

    return result;
}

void list_incref(ListData* data)
{
    data->refCount++;
}

void list_decref(ListData* data)
{
    ca_assert(data->refCount > 0);
    data->refCount--;

    if (data->refCount == 0)
        free_list(data);
}

void free_list(ListData* data)
{
    if (data == NULL)
        return;

    // Release all elements
    for (int i=0; i < data->count; i++)
        set_null(&data->items[i]);
    free(data);
}

ListData* as_list_data(Value* val)
{
    ca_assert(is_list(val));
    return (ListData*) val->value_data.ptr;
}

Value* list_get(ListData* data, int index)
{
    ca_assert(data != NULL);
    ca_assert(index < data->count);
    ca_assert(index >= 0);
    return &data->items[index];
}
Value* list_get_from_end(ListData* data, int index)
{
    ca_assert(data != NULL);
    ca_assert(index < data->count);
    ca_assert(index >= 0);
    return &data->items[data->count - index - 1];
}

ListData* list_touch(ListData* original)
{
    if (original == NULL)
        return NULL;

    if (original->refCount == 1)
        return original;

    ListData* copy = list_duplicate(original);
    list_decref(original);
    return copy;
}

ListData* list_duplicate(ListData* source)
{
    if (source == NULL)
        return NULL;

    stat_increment(ListDuplicate);

    ListData* result = allocate_list(source->count, source->capacity);
    
    if (source->count >= 100)
        stat_increment(ListDuplicate_100Count);

    for (int i=0; i < source->count; i++) {
        stat_increment(ListDuplicate_ElementCopy);
        copy(&source->items[i], &result->items[i]);
    }

    return result;
}

ListData* list_increase_capacity(ListData* original, int newCapacity)
{
    if (original == NULL)
        return allocate_list(0, newCapacity);

    bool createCopy = original->refCount > 1;

    if (createCopy) {

        ListData* result = allocate_list(original->count, newCapacity);
        for (int i=0; i < original->count; i++)
            copy(list_get(original, i), list_get(result, i));

        return result;

    } else {
        ListData* result = (ListData*) realloc(original, list_size(newCapacity));
        ca_assert(result != NULL);
        for (int i=result->capacity; i < newCapacity; i++)
            initialize_null(&result->items[i]);
        result->capacity = newCapacity;
        return result;
    }
}

ListData* list_double_capacity(ListData* original)
{
    if (original == NULL)
        return allocate_list(0, 1);

    return list_increase_capacity(original, original->capacity * 2);
}

ListData* list_resize(ListData* original, int newLength)
{
    // Check if 'original' is an empty list.
    if (original == NULL) {

        // If newLength is 0 then no change.
        if (newLength == 0)
            return NULL;

        // Create a new empty list.
        return allocate_list(newLength, newLength);
    }

    // If newLength is 0 then return an empty list.
    if (newLength == 0) {
        list_decref(original);
        return NULL;
    }

    // Check if the new length is the same as the old length.
    if (original->count == newLength)
        return original;

    ListData* result = NULL;

    if (newLength > original->capacity) {
        // Grow list
        int newCapacity = std::max(newLength, original->capacity*2);
        result = list_increase_capacity(original, newCapacity);

        // Initialize data for new elements
        for (int i=result->count; i < newLength; i++)
            initialize_null(&result->items[i]);

    } else {

        // Shrink list
        result = list_touch(original);

        // Nullify discarded elements
        for (int i=newLength; i < result->count; i++)
            set_null(&result->items[i]);
    }

    result->count = newLength;

    return result;
}

Value* list_append(ListData** dataPtr)
{
    if (*dataPtr == NULL) {
        *dataPtr = allocate_list(0, 1);
    } else {
        *dataPtr = list_touch(*dataPtr);
        
        if ((*dataPtr)->count == (*dataPtr)->capacity)
            *dataPtr = list_double_capacity(*dataPtr);
    }

    ListData* data = *dataPtr;
    data->count++;
    return &data->items[data->count - 1];
}

void list_pop(Value* list)
{
    list_resize(list, list_length(list) - 1);
}

Value* list_last(Value* list)
{
    return list_get(list, list_length(list) - 1);
}

Value* list_insert(ListData** dataPtr, int index)
{
    list_append(dataPtr);

    ListData* data = *dataPtr;

    // Move everything over, up till 'index'.
    for (int i = data->count - 1; i >= (index + 1); i--)
        swap(&data->items[i], &data->items[i - 1]);

    return &data->items[index];
}

int list_length(ListData* data)
{
    if (data == NULL)
        return 0;
    return data->count;
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

void list_copy(Value* source, Value* dest)
{
    stat_increment(ListSoftCopy);

    ca_assert(source->value_type->storageType == sym_StorageTypeList);

    make_no_initialize(source->value_type, dest);
    ListData* sourceData = (ListData*) source->value_data.ptr;

    if (sourceData == NULL)
        return;

    list_incref(sourceData);

    dest->value_data.ptr = sourceData;
}

void list_to_string(ListData* value, Value* out)
{
    if (value == NULL) {
        string_append(out, "[]");
        return;
    }

    string_append(out, "[");
    for (int i=0; i < value->count; i++) {
        if (i > 0)
            string_append(out, ", ");
        to_string(&value->items[i], out);
    }
    string_append(out, "]");
}

int list_length(Value* value)
{
    ca_assert(is_list_based(value));
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL)
        return 0;
    return s->count;
}

bool list_empty(Value* value)
{
    return list_length(value) == 0;
}

void list_slice(Value* original, int start, int end, Value* result)
{
    int originalCount = list_length(original);

    if (end < 0)
        end = originalCount - end;

    if (end > originalCount)
        end = originalCount;

    int resultCount = end - start;
    if (resultCount < 0)
        resultCount = 0;

    set_list(result, resultCount);

    for (int i=0; i < resultCount; i++)
        copy(list_get(original, i + start), list_get(result, i));
}

void list_reverse(Value* list)
{
    int count = list_length(list);
    for (int i=0; i < count/2; i++) {
        swap(list_get(list, i), list_get(list, count - i - 1));
    }
}

bool list_contains(Value* list, Value* element)
{
    for (int i=0; i < list_length(list); i++)
        if (equals(list_get(list, i), element))
            return true;

    return false;
}

bool list_strict_equals(Value* left, Value* right)
{
    if (left->value_type != right->value_type)
        return false;

    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    int count = list_length(left);
    if (count != list_length(right))
        return false;

    for (int i=0; i < count; i++)
        if (!strict_equals(list_get(left, i), list_get(right, i)))
            return false;

    // Lists are equal. Perform sneaky-equals optimization.
    set_null(right);
    list_copy(left, right);

    return true;
}

static void mergesort_step(Value* list, SortCompareFunc func, void* context)
{
    int length = list_length(list);
    int middle = int(length / 2);
    int leftLength = middle;
    int rightLength = length - middle;

    if (length <= 1 || leftLength <= 0 || rightLength <= 0)
        return;

    Value left;
    set_list(&left, leftLength);
    Value right;
    set_list(&right, rightLength);

    list_touch(list);

    // Divide

    for (int i=0; i < leftLength; i++)
        move(list_get(list, i), list_get(&left, i));
    for (int i=0; i < rightLength; i++)
        move(list_get(list, i + middle), list_get(&right, i));

    mergesort_step(&left, func, context);
    mergesort_step(&right, func, context);

    // Merge
    int leftIndex = 0;
    int rightIndex = 0;
    int destIndex = 0;

    while (true) {

        if (leftIndex >= list_length(&left)) {
            while (rightIndex < list_length(&right))
                move(list_get(&right, rightIndex++), list_get(list, destIndex++));
            return;
        }

        if (rightIndex >= list_length(&right)) {
            while (leftIndex < list_length(&left))
                move(list_get(&left, leftIndex++), list_get(list, destIndex++));
            return;
        }

        Value* leftValue = list_get(&left, leftIndex);
        Value* rightValue = list_get(&right, rightIndex);
        Value* dest = list_get(list, destIndex++);

        int compareResult = func(context, leftValue, rightValue);
        if (compareResult < 0) {
            move(leftValue, dest);
            leftIndex++;
        } else {
            move(rightValue, dest);
            rightIndex++;
        }
    }
}

static int default_compare_for_sort(void* context, Value* left, Value* right)
{
    return compare(left, right);
}

void list_sort_mergesort(Value* list, SortCompareFunc func, void* context)
{
    if (func == NULL)
        func = default_compare_for_sort;

    mergesort_step(list, func, context);
}

void list_sort(Value* list, SortCompareFunc func, void* context)
{
    list_sort_mergesort(list, func, context);
}

void list_touch(Value* value)
{
    ca_assert(is_list_based(value));
    ListData* data = (ListData*) get_pointer(value);
    set_pointer(value, list_touch(data));
}

bool list_touch_is_necessary(Value* value)
{
    ListData* data = (ListData*) get_pointer(value);
    return !(data == NULL || data->refCount == 1);
}

Value* list_get(Value* value, int index)
{
    ca_assert(is_list_based(value));
    return list_get((ListData*) value->value_data.ptr, index);
}

Value* list_get_from_end(Value* value, int reverseIndex)
{
    ca_assert(is_list_based(value));
    return list_get_from_end((ListData*) value->value_data.ptr, reverseIndex);
}
Value* list_get_safe(Value* value, int index)
{
    if (!is_list_based(value) || index < 0 || index >= list_length(value))
        return NULL;
    return list_get(value, index);
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

void list_remove_index(Value* list, int index)
{
    ca_assert(is_list(list));
    ListData* data = (ListData*) list->value_data.ptr;
    list->value_data.ptr = list_remove_index(data, index);
}

void list_resize(Value* list, int size)
{
    ca_assert(is_list_based(list));
    ListData* data = (ListData*) list->value_data.ptr;
    data = list_resize(data, size);
    list->value_data.ptr = data;
}

Value* list_append(Value* list)
{
    ca_assert(is_list(list));
    ListData* data = (ListData*) list->value_data.ptr;
    Value* result = list_append(&data);
    list->value_data.ptr = data;
    return result;
}

void list_extend(Value* list, Value* rhsList)
{
    for (int i=0; i < list_length(rhsList); i++)
        copy(list_get(rhsList, i), list_append(list));
}

Value* list_insert(Value* list, int index)
{
    ca_assert(list->value_type->storageType == sym_StorageTypeList);
    ListData* data = (ListData*) list->value_data.ptr;
    Value* result = list_insert(&data, index);
    list->value_data.ptr = data;
    return result;
}

bool list_equals(Value* left, Value* right)
{
    ca_assert(is_list_based(left));

    if (!is_list_based(right))
        return false;

    // Shortcut: lists are equal if they have the same address.
    if (left->value_data.ptr == right->value_data.ptr)
        return true;

    int leftCount = list_length(left);

    // Not equal if lengths differ.
    if (leftCount != list_length(right))
        return false;

    // Check every element.
    for (int i=0; i < leftCount; i++) {
        if (!equals(list_get(left, i), list_get(right, i)))
            return false;
    }

    return true;
}

void list_remove_and_replace_with_last_element(Value* value, int index)
{
    ca_assert(is_list(value));
    list_remove_and_replace_with_last_element((ListData**) &value->value_data, index);
}

void list_remove_nulls(Value* value)
{
    ca_assert(is_list(value));
    list_remove_nulls((ListData**) &value->value_data);
}

Symbol list_get_parameter_type(Value* parameter)
{
    if (is_null(parameter))
        return sym_Untyped;
    if (is_type(parameter))
        return sym_UniformListType;

    if (is_list(parameter)) {
        if ((list_length(parameter) == 2) && is_list(list_get(parameter, 0)))
            return sym_StructType;
        else
            return sym_AnonStructType;
    }
    return sym_Invalid;
}

bool list_type_has_specific_size(Value* parameter)
{
    return is_list(parameter);
}

Type* create_compound_type()
{
    Type* type = create_type();
    setup_compound_type(type);
    return type;
}

void setup_compound_type(Type* type)
{
    list_t::setup_type(type);
    Value* param = &type->parameter;
    set_list(param, 2);
    set_list(list_get(param, 0), 0);
    set_list(list_get(param, 1), 0);
}

void compound_type_append_field(Type* type, Type* fieldType, Value* fieldName)
{
    ca_assert(list_get_parameter_type(&type->parameter) == sym_StructType);

    list_touch(&type->parameter);
    Value* types = list_get(&type->parameter, 0);
    Value* names = list_get(&type->parameter, 1);

    set_type(list_append(types), fieldType);
    set_value(list_append(names), fieldName);
}

int compound_type_get_field_count(Type* type)
{
    Value* types = list_get(&type->parameter, 0);
    return list_length(types);
}
const char* compound_type_get_field_name(Type* type, int index)
{
    Value* names = list_get(&type->parameter, 1);
    return as_cstring(list_get(names, index));
}
Type* compound_type_get_field_type(Type* type, int index)
{
    Value* types = list_get(&type->parameter, 0);
    return as_type(list_get(types, index));
}

bool is_struct_type(Type* type)
{
    return list_get_parameter_type(&type->parameter) == sym_StructType;
}

void compound_type_to_string(Value* value, Value* out)
{
    if (!is_string(out))
        set_string(out, "");

    Type* type = value->value_type;
    
    string_append(out, "{");
    for (int i=0; i < compound_type_get_field_count(type); i++) {
        if (i != 0)
            string_append(out, ", ");

        const char* name = compound_type_get_field_name(type, i);
        string_append(out, name);
        string_append(out, ": ");
        string_append_quoted(out, list_get(value, i));
    }
    string_append(out, "}");
}

void list_type_initialize_from_decl(Type* type, Block* decl)
{
    setup_compound_type(type);

    // Iterate through the type definition.
    for (int i=0; i < decl->length(); i++) {
        Term* term = decl->get(i);

        if (!is_function(term) || !term->boolProp(sym_FieldAccessor, false))
            continue;

        Type* fieldType = get_output_type(nested_contents(term), 0);

        compound_type_append_field(type, fieldType, &term->nameValue);
    }
}

Value* list_get_type_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    Value* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case sym_AnonStructType:
        return parameter;
    case sym_StructType:
        return list_get(parameter, 0);
    case sym_Untyped:
    case sym_UniformListType:
    case sym_Invalid:
        return NULL;
    }
    ca_assert(false);
    return NULL;
}

Value* list_get_name_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    Value* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case sym_StructType:
        return list_get(parameter, 1);
    case sym_AnonStructType:
    case sym_Untyped:
    case sym_UniformListType:
    case sym_Invalid:
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
int list_find_field_index_by_name(Type* listType, const char* name)
{
    if (!is_list_based_type(listType))
        return -1;

    Value* names = list_get_name_list_from_type(listType);
    if (names == NULL)
        return -1;

    for (int i=0; i < circa_count(names); i++)
        if (string_equals(circa_index(names, i), name))
            return i;

    // Not found
    return -1;
}

bool is_list_based_type(Type* type)
{
    return type->storageType == sym_StorageTypeList;
}

namespace list_t {

    void resize(Value* list, int newSize)
    {
        ca_assert(is_list(list));
        set_pointer(list, list_resize((ListData*) get_pointer(list), newSize));
    }

    void tv_initialize(Type* type, Value* value)
    {
        ca_assert(value->value_data.ptr == NULL);

        // If the parameter has a fixed size list, then initialize to that.
        if (list_type_has_specific_size(&type->parameter)) {
            Value* typeList = list_get_type_list_from_type(type);
            int count = list_length(typeList);

            list_resize(value, count);
            for (int i=0; i < count; i++)
                make(as_type(list_get(typeList, i)), list_get(value, i));
        }
    }

    void tv_release(Value* value)
    {
        ca_assert(is_list_based(value));
        ListData* data = (ListData*) get_pointer(value);
        if (data == NULL)
            return;
        list_decref(data);
    }

    void tv_copy(Value* source, Value* dest)
    {
        list_copy(source, dest);
    }

    void tv_cast(CastResult* result, Value* value, Type* type, bool checkOnly)
    {
        ca_assert(value->value_type != type);

        if (!is_list_based(value)) {
            result->success = false;
            return;
        }

        int sourceLength = list_length(value);

        // If the requested type doesn't have a specific size restriction, then
        // the input data is fine as-is.
        if (!list_type_has_specific_size(&type->parameter)) {
            if (!checkOnly) {
                type_incref(type);
                type_decref(value->value_type);
                value->value_type = type;
            }
            return;
        }

        Value* destTypes = list_get_type_list_from_type(type);

        // Check for correct number of elements.
        if (sourceLength != list_length(destTypes)) {
            result->success = false;
            return;
        }

        if (!checkOnly) {
            if (touch_is_necessary(value))
                stat_increment(ListCast_Touch);

            list_touch(value);
            type_incref(type);
            type_decref(value->value_type);
            value->value_type = type;
        }

        for (int i=0; i < sourceLength; i++) {
            Value* sourceElement = list_get(value, i);
            Type* expectedType = as_type(list_get(destTypes,i));

            stat_increment(ListCast_CastElement);
            cast(result, sourceElement, expectedType, checkOnly);

            if (!result->success)
                return;
        }
    }

    void tv_set_index(Value* value, int index, Value* element)
    {
        ca_assert(is_list_based(value));
        list_touch(value);
        copy(element, list_get(value, index));
    }

    void tv_to_string(Value* value, Value* out)
    {
        if (is_struct_type(value->value_type))
            return compound_type_to_string(value, out);

        return list_to_string((ListData*) get_pointer(value), out);
    }


    u32 circular_shift(u32 value, int shift)
    {
        shift = shift % 32;
        if (shift == 0)
            return value;
        else
            return (value << shift) | (value >> (32 - shift));
    }
    
    int list_hash(Value* value)
    {
        int hash = 0;
        int count = list_length(value);
        for (int i=0; i < count; i++) {
            int itemHash = get_hash_value(list_get(value, i));
            hash ^= circular_shift(itemHash, i);
        }
        return hash;
    }

    void tv_static_type_query(Type* type, StaticTypeQuery* query)
    {
        Term* subject = query->subject;
        Type* subjectType = query->subjectType;

        // If the type doesn't have a specific size, then accept any list.
        if (!list_type_has_specific_size(&type->parameter)) {
            if (is_list_based_type(subjectType))
                return query->succeed();
            else
                return query->fail();
        }

        Value* expectedElementTypes = list_get_type_list_from_type(type);

        // Special case when looking at a call to list(); look at inputs instead of
        // looking at the result.
        if (subject && subject->function == FUNCS.list)
        {
            if (subject->numInputs() != circa_count(expectedElementTypes))
                return query->fail();

            for (int i=0; i < circa_count(expectedElementTypes); i++)
                if (!circa::term_output_always_satisfies_type(
                            subject->input(i), as_type(circa_index(expectedElementTypes, i))))
                    return query->fail();

            return query->succeed();
        }

        // Look at the subject's type.
        if (!list_type_has_specific_size(&subjectType->parameter))
            return query->fail();

        Value* subjectElementTypes = list_get_type_list_from_type(subjectType);

        bool anyUnableToDetermine = false;

        for (int i=0; i < circa_count(expectedElementTypes); i++) {
            if (i >= circa_count(subjectElementTypes))
                return query->fail();

            StaticTypeQuery::Result result = run_static_type_query(
                    as_type(circa_index(expectedElementTypes,i)),
                    as_type(circa_index(subjectElementTypes,i)));

            // If any of these fail, then fail.
            if (result == StaticTypeQuery::FAIL)
                return query->fail();

            if (result == StaticTypeQuery::UNABLE_TO_DETERMINE)
                anyUnableToDetermine = true;
        }

        if (anyUnableToDetermine)
            return query->unableToDetermine();
        else
            return query->succeed();
    }

    void tv_visit_heap(Type*, Value* value, Type::VisitHeapCallback callback, Value* context)
    {
        ListData* data = (ListData*) value->value_data.ptr;
        if (data == NULL)
            return;
        Value relativeIdentifier;
        for (int i=0; i < data->count; i++) {
            set_int(&relativeIdentifier, i);
            callback(&data->items[i], &relativeIdentifier, context);
        }
    }

    void setup_type(Type* type)
    {
        if (string_equals(&type->name, ""))
            set_string(&type->name, "List");
        type->storageType = sym_StorageTypeList;
        type->initialize = tv_initialize;
        type->release = tv_release;
        type->copy = tv_copy;
        type->toString = tv_to_string;
        type->equals = list_equals;
        type->cast = tv_cast;
        type->getIndex = list_get;
        type->setIndex = tv_set_index;
        type->numElements = list_length;
        type->hashFunc = list_hash;
        type->staticTypeQuery = tv_static_type_query;
        type->visitHeap = tv_visit_heap;
    }

} // namespace list_t

} // namespace circa
