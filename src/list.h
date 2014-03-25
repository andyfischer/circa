// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "tagged_value.h"

#ifdef _MSC_VER

// Tell visual studio not to complain about zero-length array.
#pragma warning(disable:4200)
#pragma warning(disable:4624)

#endif

namespace circa {

struct ListData {
    int refCount;
    int count;
    int capacity;
    bool immutable;
    int checksum;

    // items has size [capacity].
    caValue items[0];

    // for debugging:
    void dump();
};

// Functions for dealing directly with a ListData object

void assert_valid_list(ListData* list);
ListData* allocate_empty_list(int capacity);
ListData* allocate_list(int num_elements);
void list_decref(ListData* data);
void free_list(ListData* data);
void list_make_immutable(ListData* data);
ListData* as_list_data(caValue* val);

caValue* list_get(ListData* data, int index);
caValue* list_get_from_end(ListData* data, int index);
int list_length(ListData* data);
caValue* list_append(ListData** dataPtr);
caValue* list_insert(ListData** dataPtr, int index);
ListData* list_touch(ListData* original);
ListData* list_duplicate(ListData* source);
ListData* list_increase_capacity(ListData* original, int new_capacity);
ListData* list_double_capacity(ListData* original);
ListData* list_resize(ListData* original, int numElements);

// Functions for dealing with a list inside a caValue container

// Signal that we are about to start modifying the internals of the list.
// (for example, modifying a result of list_get). If the value currently
// holds an immutable shared copy, then this function will create a
// duplicate list which is safe to modify.
void list_touch(caValue* list);

// Get an element by index. If the caller plans to modify the returned value,
// or pass the value somewhere that may modify it (without making a copy),
// then they must call list_touch before list_get.
caValue* list_get(caValue* value, int index);

// Get an element by index, counting from the end. For example, asking for
// element 0 returns the last element. See caveats for list_get.
caValue* list_get_from_end(caValue* value, int index);

caValue* list_get_safe(caValue* value, int index);

// Return the number of elements in the list.
int list_length(caValue* value);

bool list_empty(caValue* value);

// Appends an element to the list and returns it. This function calls
// list_touch, so the returned value is safe to modify.
caValue* list_append(caValue* list);

void list_pop(caValue* list);

caValue* list_last(caValue* list);

// Appends each element from the given right-hand-side.
void list_extend(caValue* list, caValue* rhsList);

// Inserts a new value at the given index, and returns it. This function
// calls list_touch, so the returned value is safe to modify.
caValue* list_insert(caValue* list, int index);

void list_remove_index(caValue* list, int index);
void list_resize(caValue* list, int size);
void list_remove_and_replace_with_last_element(caValue* list, int index);
void list_remove_nulls(caValue* list);

// Set 'dest' to be a copy of 'source'. This will cause 'source' to be immutable
// (if it wasn't already).
void list_copy(caValue* source, caValue* dest);

std::string list_to_string(ListData* value);
void list_slice(caValue* original, int start, int end, caValue* result);

// Reverse the list.
void list_reverse(caValue* list);

// Return true if any of the elements of the list are equal to this element.
bool list_contains(caValue* list, caValue* element);

bool list_strict_equals(caValue* left, caValue* right);

typedef int (*SortCompareFunc)(void* context, caValue* left, caValue* right);
void list_sort_mergesort(caValue* list, SortCompareFunc func, void* context);

// Sort the list using default method (currently mergesort). The compareFunc can
// be NULL in order to use the default compare() function.
void list_sort(caValue* list, SortCompareFunc sortFunc, void* context);

// Functions for working with List's type parameter. Depending on the parameter,
// the list can be untyped, typed with an arbitrary size, or typed with
// a specific size.
Symbol list_get_parameter_type(caValue* parameter);
bool list_type_has_specific_size(caValue* parameter);
void list_type_initialize_from_decl(Type* type, Block* decl);

// Create a new compound type (also called a struct type)
Type* create_compound_type();

void setup_compound_type(Type* type);

void compound_type_append_field(Type* type, Type* fieldType, caValue* fieldName);

// For a List-based type, this returns the number of elements.
int compound_type_get_field_count(Type* type);

const char* compound_type_get_field_name(Type* type, int index);

// For a List-based type, retrieve the type of the given field index.
Type* compound_type_get_field_type(Type* listType, int index);

bool is_struct_type(Type* type);

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

// For a List-based type, this returns the index of the field with the given name.
// Returns -1 if the field is not found.
int list_find_field_index_by_name(Type* listType, const char* name);

bool is_list_based_type(Type*);

namespace list_t {

    void setup_type(Type*);
    void tv_initialize(Type*, caValue*);
    std::string tv_to_string(caValue* value);
}

} // namespace circa
