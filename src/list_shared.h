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
    TValue items[0];
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
TValue* list_append(ListData** dataPtr);
TValue* list_insert(ListData** dataPtr, int index);
int list_size(ListData* data);
TValue* list_get_index(ListData* data, int index);
void list_set_index(ListData* data, int index, TValue* value);

int list_get_length(TValue* value);
int list_size(TValue* value);
int list_length(TValue* value);
TValue* list_get(TValue* value, int index);
TValue* list_get_index(TValue* value, int index);
TValue* list_get_index_from_end(TValue* value, int index);
void list_remove_index(TValue* list, int index);
void list_resize(TValue* list, int size);
TValue* list_append(TValue* list);
TValue* list_insert(TValue* list, int index);
void list_remove_and_replace_with_last_element(TValue* list, int index);
void list_remove_nulls(TValue* list);
std::string list_to_string(ListData* value);
void list_slice(TValue* original, int start, int end, TValue* result);

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

ListType list_get_parameter_type(TValue* parameter);
bool list_type_has_specific_size(TValue* parameter);
void list_initialize_parameter_from_type_decl(Branch* typeDecl, TValue* parameter);

// For a List-based type, this returns the list of types that the elements will
// have. This result is only valid for fixed-size List types, otherwise it will
// return NULL.
TValue* list_get_type_list_from_type(Type* type);

// For a List-based type, this returns the list of element names. This is only
// valid for a typed_sized_named list, otherwise it will return NULL.
TValue* list_get_name_list_from_type(Type* type);

// For a List-based type, this returns the type of each element. This is only
// valid for a typed_unsized list.
Type* list_get_repeated_type_from_type(Type* type);

int list_find_field_index_by_name(Type* listType, std::string const& name);

} // namespace circa
