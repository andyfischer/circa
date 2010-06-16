// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {
namespace tagged_value_vector {

struct ListData;

void assert_valid_list(ListData* list);

// Increment refcount on this list
void incref(ListData* data);

// Decrement refcount. This might cause this list to be deleted, don't
// use this data after you have given up your reference.
void decref(ListData* data);

// Modify a list so that it has the given number of elements, returns the new
// list data.
ListData* resize(ListData* original, int numElements);

// Create a new list, starts off with 1 ref.
ListData* create_list(int capacity);

// Creates a new list that is a duplicate of source. Starts off with 1 ref.
// Does not decref source.
ListData* duplicate(ListData* source);

// Returns a new list with the given capacity. Decrefs original.
ListData* increase_capacity(ListData* original, int new_capacity);

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
ListData* grow_capacity(ListData* original);

// Reset this list to have 0 elements.
void clear(ListData** data);

// Return a version of this list which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
ListData* mutate(ListData* data);

// Add a new blank element to the end of the list, resizing if necessary.
// Returns the new element.
TaggedValue* append(ListData** data);

TaggedValue* get_index(ListData* data, int index);

void set_index(ListData** data, int index, TaggedValue* v);

int num_elements(ListData* data);

// Remove the element at index and replace the empty spot with the last
// element in the list.
void remove_and_replace_with_back(ListData** data, int index);

std::string to_string(ListData* value);

int refcount(ListData* value);

}
}
