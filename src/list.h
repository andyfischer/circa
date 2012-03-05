// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "gc.h"
#include "tagged_value.h"

namespace circa {

struct ListData {
    int refCount;
    int count;
    int capacity;
    caValue items[0];
    // items has size [capacity].

    std::string toStr();
};

void assert_valid_list(ListData* list);
ListData* allocate_empty_list(int capacity);
ListData* allocate_list(int num_elements);
void list_decref(ListData* data);
void free_list(ListData* data);
ListData* list_touch(ListData* original);
ListData* list_duplicate(ListData* source);
ListData* list_increase_capacity(ListData* original, int new_capacity);
ListData* list_double_capacity(ListData* original);
ListData* list_resize(ListData* original, int numElements);
caValue* list_append(ListData** dataPtr);
caValue* list_insert(ListData** dataPtr, int index);
int list_size(ListData* data);
caValue* list_get_index(ListData* data, int index);
void list_set_index(ListData* data, int index, caValue* value);

int list_get_length(caValue* value);
int list_size(caValue* value);
int list_length(caValue* value);
caValue* list_get(caValue* value, int index);
caValue* list_get_index(caValue* value, int index);
caValue* list_get_index_from_end(caValue* value, int index);
void list_remove_index(caValue* list, int index);
void list_resize(caValue* list, int size);
caValue* list_append(caValue* list);
caValue* list_insert(caValue* list, int index);
void list_remove_and_replace_with_last_element(caValue* list, int index);
void list_remove_nulls(caValue* list);
std::string list_to_string(ListData* value);
void list_slice(caValue* original, int start, int end, caValue* result);

// Functions for working with List's type parameter. Depending on the parameter,
// the list can be untyped, typed with an arbitrary size, or typed with
// a specific size.

enum ListType
{
    LIST_INVALID_PARAMETER=0,
    LIST_UNTYPED,
    LIST_TYPED_UNSIZED,
    LIST_TYPED_SIZED,
    LIST_TYPED_SIZED_NAMED
};

ListType list_get_parameter_type(caValue* parameter);
bool list_type_has_specific_size(caValue* parameter);
void list_initialize_parameter_from_type_decl(Branch* typeDecl, caValue* parameter);

// For a List-based type, this returns the list of types that the elements will
// have. This result is only valid for fixed-size List types, otherwise it will
// return NULL.
caValue* list_get_type_list_from_type(Type* type);

// For a List-based type, this returns the list of element names. This is only
// valid for a typed_sized_named list, otherwise it will return NULL.
caValue* list_get_name_list_from_type(Type* type);

// For a List-based type, this returns the type of each element. This is only
// valid for a typed_unsized list.
Type* list_get_repeated_type_from_type(Type* type);

int list_find_field_index_by_name(Type* listType, std::string const& name);

bool is_list_based_type(Type*);

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, caValue*);
    std::string tv_to_string(caValue* value);
    caValue* append(caValue* list);
    caValue* prepend(caValue* list);
}

// Wrapper type to use a caValue as a List.
struct List : caValue
{
    List();

    caValue* append();
    caValue* prepend();
    void append(caValue* val);
    caValue* insert(int index);
    void clear();
    int length();
    bool empty();
    caValue* get(int index);
    void set(int index, caValue* value);
    caValue* operator[](int index) { return get(index); }
    void resize(int size);

    // get the item at length - 1
    caValue* getLast();

    // remove the item at length - 1
    void pop();

    // remove the item at the given index
    void remove(int index);

    void removeNulls();

    // Convenience methods
    void appendString(const char* str);
    void appendString(const std::string& str);

    static List* checkCast(caValue* v);
    static List* lazyCast(caValue* v);
    static List* cast(caValue* v, int length);
};

} // namespace circa
