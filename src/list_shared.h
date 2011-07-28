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
    TaggedValue items[0];
    // items has size [capacity].
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
TaggedValue* list_append(ListData** dataPtr);
TaggedValue* list_insert(ListData** dataPtr, int index);
TaggedValue* list_get_index(ListData* data, int index);
void list_set_index(ListData* data, int index, TaggedValue* value);

int list_get_length(TaggedValue* value);
TaggedValue* list_get_index(TaggedValue* value, int index);
TaggedValue* list_get_index_from_end(TaggedValue* value, int index);
void list_remove_index(TaggedValue* list, int index);
TaggedValue* list_append(TaggedValue* list);
TaggedValue* list_insert(TaggedValue* list, int index);
void list_remove_and_replace_with_last_element(TaggedValue* list, int index);
void list_remove_nulls(TaggedValue* list);

// Functions for working with List's parameter. Depending on the parameter,
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

ListType list_get_parameter_type(TaggedValue* parameter);
bool list_type_has_specific_size(TaggedValue* parameter);
void list_initialize_parameter_from_type_decl(Branch& typeDecl, TaggedValue* parameter);

// From the parameter of a List type value, this returns a list of types for
// the fields of the list. Only valid if this parameter describes a fixed-size
// list (otherwise it returns NULL).
TaggedValue* list_get_type_list_from_parameter(TaggedValue* parameter);

TaggedValue* list_get_name_list_from_parameter(TaggedValue* parameter);

Type* list_get_single_type_from_parameter(TaggedValue* parameter);

Type* create_typed_unsized_list(Type* elementType);

Type* get_common_typed_unsized_list(Type* elementType1, Type* elementType2);

} // namespace circa
