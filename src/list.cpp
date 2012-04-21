// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "heap_debugging.h"
#include "kernel.h"
#include "list.h"
#include "string_type.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

void ListData::dump()
{
    std::string s = list_to_string(this);
    printf("%s\n", s.c_str());
}

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
    ListData* result = (ListData*) malloc(sizeof(ListData) + capacity * sizeof(caValue));
    debug_register_valid_object(result, LIST_OBJECT);

    result->refCount = 1;
    result->count = 0;
    result->capacity = capacity;
    result->immutable = false;
    memset(result->items, 0, capacity * sizeof(caValue));
    for (int i=0; i < capacity; i++)
        initialize_null(&result->items[i]);

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
}

void list_incref(ListData* data)
{
    assert_valid_list(data);
    data->refCount++;
}

void free_list(ListData* data)
{
    if (data == NULL)
        return;

    // Release all elements
    for (int i=0; i < data->count; i++)
        set_null(&data->items[i]);
    free(data);
    debug_unregister_valid_object(data, LIST_OBJECT);
}

void list_make_immutable(ListData* data)
{
    data->immutable = true;
}

caValue* list_get(ListData* data, int index)
{
    if (data == NULL)
        return NULL;
    if (index >= data->count)
        return NULL;
    return &data->items[index];
}

ListData* list_touch(ListData* original)
{
    if (original == NULL)
        return NULL;

    if (!original->immutable)
        return original;

    ListData* copy = list_duplicate(original);
    list_decref(original);
    return copy;
}

ListData* list_duplicate(ListData* source)
{
    if (source == NULL)
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
        caValue* left = &original->items[i];
        caValue* right = &result->items[i];
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
    // Check if 'original' is an empty list
    if (original == NULL) {
        if (numElements == 0)
            return NULL;
        ListData* result = allocate_empty_list(numElements);
        result->count = numElements;
        return result;
    }

    // Check to reduce 'original' to an empty list
    if (numElements == 0) {
        list_decref(original);
        return NULL;
    }

    // Check if the number of elements doesn't need to change
    if (original->count == numElements)
        return original;

    // Check for not enough capacity
    if (numElements > original->capacity) {
        ListData* result = list_increase_capacity(original, numElements);
        result->count = numElements;
        return result;
    }

    // Capacity is good, will need to modify 'count' on list and possibly
    // set some items to null. This counts as a modification.
    ListData* result = list_touch(original);

    // Possibly set extra elements to null, if we are shrinking.
    for (int i=numElements; i < result->count; i++)
        set_null(&result->items[i]);
    result->count = numElements;

    return result;
}

caValue* list_append(ListData** dataPtr)
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

caValue* list_insert(ListData** dataPtr, int index)
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

void list_copy(caValue* source, caValue* dest)
{
    ca_assert(source->value_type->storageType == STORAGE_TYPE_LIST);

    // prepare 'dest'
    change_type(dest, source->value_type);

    ListData* sourceData = (ListData*) source->value_data.ptr;

    if (sourceData == NULL)
        return;

    list_make_immutable(sourceData);
    list_incref(sourceData);

    dest->value_data.ptr = sourceData;
}

std::string list_to_string(ListData* value)
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

int list_length(caValue* value)
{
    ListData* s = (ListData*) get_pointer(value);
    if (s == NULL)
        return 0;
    return s->count;
}

bool list_empty(caValue* value)
{
    return list_length(value) == 0;
}

void list_slice(caValue* original, int start, int end, caValue* result)
{
    if (end < 0)
        end = list_length(original) - end;

    int count = end - start;
    set_list(result, count);

    for (int i=0; i < count; i++)
        copy(list_get(original, i + start), list_get(result, i));
}

caValue* list_get(caValue* value, int index)
{
    ca_assert(value->value_type->storageType == STORAGE_TYPE_LIST);
    return list_get((ListData*) value->value_data.ptr, index);
}

caValue* list_get_from_end(caValue* value, int reverseIndex)
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

void list_remove_index(caValue* list, int index)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    list_remove_index(data, index);
    list->value_data.ptr = data;
}

void list_resize(caValue* list, int size)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    data = list_resize(data, size);
    list->value_data.ptr = data;
}

caValue* list_append(caValue* list)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    caValue* result = list_append(&data);
    list->value_data.ptr = data;
    return result;
}
caValue* list_insert(caValue* list, int index)
{
    ca_assert(list->value_type->storageType == STORAGE_TYPE_LIST);
    ListData* data = (ListData*) list->value_data.ptr;
    caValue* result = list_insert(&data, index);
    list->value_data.ptr = data;
    return result;
}

void list_touch(caValue* list)
{
    list->value_data.ptr = (ListData*) list_touch((ListData*) list->value_data.ptr);
}

void list_remove_and_replace_with_last_element(caValue* value, int index)
{
    ca_assert(is_list(value));
    list_remove_and_replace_with_last_element((ListData**) &value->value_data, index);
}

void list_remove_nulls(caValue* value)
{
    ca_assert(is_list(value));
    list_remove_nulls((ListData**) &value->value_data);
}

ListType list_get_parameter_type(caValue* parameter)
{
    if (is_null(parameter))
        return LIST_UNTYPED;
    if (is_type(parameter))
        return LIST_TYPED_UNSIZED;

    if (is_list(parameter)) {
        if ((list_length(parameter) == 2) && is_list(list_get(parameter, 0)))
            return LIST_TYPED_SIZED_NAMED;
        else
            return LIST_TYPED_SIZED;
    }
    return LIST_INVALID_PARAMETER;
}

bool list_type_has_specific_size(caValue* parameter)
{
    return is_list(parameter);
}

Type* create_compound_type()
{
    Type* type = create_type();

    list_t::setup_type(type);
    List* param = set_list(&type->parameter, 2);
    set_list(param->get(0), 0);
    set_list(param->get(1), 0);
    return type;
}

void compound_type_append_field(Type* type, Type* fieldType, const char* fieldName)
{
    ca_assert(list_get_parameter_type(&type->parameter) == LIST_TYPED_SIZED_NAMED);

    list_touch(&type->parameter);
    caValue* types = list_get(&type->parameter, 0);
    caValue* names = list_get(&type->parameter, 1);

    set_type(list_append(types), fieldType);
    set_string(list_append(names), fieldName);
}

int compound_type_get_field_count(Type* type)
{
    caValue* types = list_get(&type->parameter, 0);
    return list_length(types);
}
const char* compound_type_get_field_name(Type* type, int index)
{
    caValue* names = list_get(&type->parameter, 1);
    return as_cstring(list_get(names, index));
}
Type* compound_type_get_field_type(Type* type, int index)
{
    caValue* types = list_get(&type->parameter, 0);
    return as_type(list_get(types, index));
}

bool is_compound_type(Type* type)
{
    return list_get_parameter_type(&type->parameter) == LIST_TYPED_SIZED_NAMED;
}

std::string compound_type_to_string(caValue* value)
{
    Type* type = value->value_type;

    std::stringstream out;
    out << "{";
    for (int i=0; i < compound_type_get_field_count(type); i++) {
        if (i != 0)
            out << ", ";

        const char* name = compound_type_get_field_name(type, i);
        out << name;
        out << ": ";
        out << to_string(list_get(value, i));
    }
    out << "}";
    return out.str();
}

void list_initialize_parameter_from_type_decl(Branch* typeDecl, caValue* parameter)
{
    List& param = *set_list(parameter, 2);
    List& types = *set_list(param[0], typeDecl->length());
    List& names = *set_list(param[1], typeDecl->length());

    for (int i=0; i < typeDecl->length(); i++) {
        set_type(types[i], declared_type(typeDecl->get(i)));
        set_string(names[i], typeDecl->get(i)->name);
    }
}

caValue* list_get_type_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    caValue* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case LIST_TYPED_SIZED:
        return parameter;
    case LIST_TYPED_SIZED_NAMED:
        return list_get(parameter, 0);
    case LIST_UNTYPED:
    case LIST_TYPED_UNSIZED:
    case LIST_INVALID_PARAMETER:
        return NULL;
    }
    ca_assert(false);
    return NULL;
}

caValue* list_get_name_list_from_type(Type* type)
{
    ca_assert(is_list_based_type(type));
    caValue* parameter = &type->parameter;

    switch (list_get_parameter_type(parameter)) {
    case LIST_TYPED_SIZED_NAMED:
        return list_get(parameter, 1);
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
int list_find_field_index_by_name(Type* listType, const char* name)
{
    if (!is_list_based_type(listType))
        return -1;

    caValue* nameList = list_get_name_list_from_type(listType);
    if (nameList == NULL)
        return -1;

    List& names = *as_list(nameList);
    for (int i=0; i < names.length(); i++)
        if (string_eq(names[i], name))
            return i;

    // Not found
    return -1;
}

bool is_list_based_type(Type* type)
{
    return type->initialize == list_t::tv_initialize;
}

namespace list_t {

    // caValue wrappers
    void tv_touch(caValue* value);

    void resize(caValue* list, int newSize)
    {
        ca_assert(is_list(list));
        set_pointer(list, list_resize((ListData*) get_pointer(list), newSize));
    }

    void tv_initialize(Type* type, caValue* value)
    {
        ca_assert(value->value_data.ptr == NULL);

        // If the parameter has a fixed size list, then initialize to that.
        if (list_type_has_specific_size(&type->parameter)) {
            caValue* typeList = list_get_type_list_from_type(type);
            List& types = *List::checkCast(typeList);
            List& result = *List::checkCast(value);
            result.resize(types.length());
            for (int i=0; i < types.length(); i++)
                create(as_type(types[i]), result[i]);
        }
    }

    void tv_release(caValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        if (data == NULL)
            return;
        list_decref(data);
    }

    void tv_copy(Type* type, caValue* source, caValue* dest)
    {
        list_copy(source, dest);
    }

    bool tv_equals(Type*, caValue* leftcaValue, caValue* right)
    {
        ca_assert(is_list(leftcaValue));
        Type* rhsType = right->value_type;
        if (rhsType->numElements == NULL || rhsType->getIndex == NULL)
            return false;

        List* left = List::checkCast(leftcaValue);

        int leftCount = left->numElements();

        if (leftCount != right->numElements())
            return false;

        for (int i=0; i < leftCount; i++) {
            if (!circa::equals(left->get(i), right->getIndex(i)))
                return false;
        }
        return true;
    }

    void tv_cast(CastResult* result, caValue* source, Type* type,
        caValue* dest, bool checkOnly)
    {
        List* sourceList = List::checkCast(source);

        if (sourceList == NULL) {
            result->success = false;
            return;
        }

        int sourceLength = sourceList->length();

        // If the destination list type doesn't have a specific size restriction,
        // then just copy the source list and call it a day.
        if (!list_type_has_specific_size(&type->parameter)) {
            if (!checkOnly) {
                copy(source, dest);
                dest->value_type = type;
            }
            return;
        }

        List& destTypes = *List::checkCast(list_get_type_list_from_type(type));

        // Check for correct number of elements.
        if (sourceLength != destTypes.length()) {
            result->success = false;
            return;
        }

        List* destList = NULL;

        if (!checkOnly) {
            destList = List::lazyCast(dest);
            destList->resize(sourceLength);
            dest->value_type = type;
        }

        for (int i=0; i < sourceLength; i++) {
            caValue* sourceElement = sourceList->get(i);
            Type* expectedType = as_type(destTypes[i]);

            caValue* destElement = NULL;
            if (!checkOnly)
                destElement = destList->get(i);
        
            cast(result, sourceElement, expectedType, destElement, checkOnly);

            if (!result->success)
                return;
        }

        if (!checkOnly)
            dest->value_type = type;
    }

    void tv_set_index(caValue* value, int index, caValue* element)
    {
        ca_assert(is_list(value));
        list_touch(value);
        copy(element, list_get(value, index));
    }

    caValue* tv_get_field(caValue* value, const char* fieldName)
    {
        int index = list_find_field_index_by_name(value->value_type, fieldName);
        if (index < 0)
            return NULL;
        return list_get(value, index);
    }

    std::string tv_to_string(caValue* value)
    {
        ca_assert(is_list(value));
        if (is_compound_type(value->value_type))
            return compound_type_to_string(value);

        return list_to_string((ListData*) get_pointer(value));
    }

    void tv_touch(caValue* value)
    {
        ca_assert(is_list(value));
        ListData* data = (ListData*) get_pointer(value);
        set_pointer(value, list_touch(data));
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

        List& expectedElementTypes = *as_list(list_get_type_list_from_type(type));

        // Special case when looking at a call to list(); look at inputs instead of
        // looking at the result.
        if (subject && subject->function == FUNCS.list)
        {
            if (subject->numInputs() != expectedElementTypes.length())
                return query->fail();

            for (int i=0; i < expectedElementTypes.length(); i++)
                if (!circa::term_output_always_satisfies_type(
                            subject->input(i), as_type(expectedElementTypes[i])))
                    return query->fail();

            return query->succeed();
        }

        // Look at the subject's type.
        if (!list_type_has_specific_size(&subjectType->parameter))
            return query->fail();

        List& subjectElementTypes = *as_list(list_get_type_list_from_type(subjectType));

        bool anyUnableToDetermine = false;

        for (int i=0; i < expectedElementTypes.length(); i++) {
            if (i >= subjectElementTypes.length())
                return query->fail();

            StaticTypeQuery::Result result = run_static_type_query(
                    as_type(expectedElementTypes[i]), as_type(subjectElementTypes[i]));

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

    void tv_visit_heap(Type*, caValue* value, Type::VisitHeapCallback callback, caValue* context)
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
        type->name = name_from_string("List");
        type->storageType = STORAGE_TYPE_LIST;
        type->initialize = tv_initialize;
        type->release = tv_release;
        type->copy = tv_copy;
        type->toString = tv_to_string;
        type->equals = tv_equals;
        type->cast = tv_cast;
        type->getIndex = list_get;
        type->setIndex = tv_set_index;
        type->getField = tv_get_field;
        type->numElements = list_length;
        type->touch = tv_touch;
        type->staticTypeQuery = tv_static_type_query;
        type->visitHeap = tv_visit_heap;
    }

} // namespace list_t


List::List()
  : Value()
{
    create(&LIST_T, this);
}

caValue*
List::append()
{
    return list_append(this);
}
void
List::append(caValue* val)
{
    copy(val, append());
}

caValue* List::insert(int index)
{
    return list_insert((ListData**) &this->value_data.ptr, index);
}

caValue*
List::prepend()
{
    return list_insert(this, 0);
}

void
List::clear()
{
    list_resize(this, 0);
}

int
List::length()
{
    return list_length(this);
}

bool
List::empty()
{
    return length() == 0;
}

caValue*
List::get(int index)
{
    return list_get(this, index);
}

void
List::set(int index, caValue* value)
{
    list_touch(this);
    copy(value, list_get(this, index));
}

void
List::resize(int newSize)
{
    list_resize(this, newSize); 
}

caValue*
List::getLast()
{
    return get(length() - 1);
}

void
List::pop()
{
    resize(length() - 1);
}

void
List::remove(int index)
{
    list_remove_index(this, index);
}

void
List::removeNulls()
{
    list_remove_nulls(this);
}
void
List::appendString(const char* str)
{
    Value val;
    set_string(&val, str);
    swap(&val, append());
}
void
List::appendString(const std::string& str)
{
    Value val;
    set_string(&val, str);
    swap(&val, append());
}

List*
List::checkCast(caValue* v)
{
    if (is_list(v))
        return (List*) v;
    else
        return NULL;
}

List*
List::lazyCast(caValue* v)
{
    if (!is_list(v))
        set_list(v, 0);
    return (List*) v;
}
List*
List::cast(caValue* v, int length)
{
    set_list(v, length);
    return (List*) v;
}

} // namespace circa
