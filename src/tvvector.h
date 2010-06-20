// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa { namespace tvvector {

struct ListData;

// Increment refcount on this list
void incref(ListData* data);

// Decrement refcount. This might cause this list to be deleted, caller
// shouldn't use 'data' after this call.
void decref(ListData* data);

// Modify a list so that it has the given number of elements, returns the new
// list data. Caller should not use 'original' pointer after this call.
ListData* resize(ListData* original, int numElements);

// Create a new list with the given capacity, starts off with 1 ref.
ListData* create_list(int capacity);

// Creates a new list that is a duplicate of source. Starts off with 1 ref.
// Does not decref source.
ListData* duplicate(ListData* source);

// Returns a new list with the given capacity, decrefs original. Caller shouldn't
// use 'original' after this call.
ListData* increase_capacity(ListData* original, int new_capacity);

// Returns a new list that has 2x the capacity of 'original', and decrefs 'original'.
// Caller shouldn't use 'original' after this call.
ListData* double_capacity(ListData* original);

// Reset this list to have 0 elements.
void clear(ListData** data);

// Return a version of this list which is safe to modify. If this data has
// multiple references, we'll return a new copy (and decref the original).
// Caller shouldn't use 'original' after this call.
ListData* touch(ListData* original);

// Add a new blank element to the end of the list, resizing if necessary.
// Returns the new element.
TaggedValue* append(ListData** data);

// Get the value at the given index. Returns NULL if the index is out of bounds.
// Caller must not modify the result value unless this list comes as a result
// of touch(), or if they know that they have the only reference.
TaggedValue* get_index(ListData* list, int index);

// Set the value at the given index to 'value'.
void set_index(ListData** data, int index, TaggedValue* value);

// Get the number of elements in this list.
int num_elements(ListData* list);

// Get the reference count of this list.
int refcount(ListData* list);

// Remove the element at index, and replace the empty spot with the last
// element in the list. This is a quick way to remove an element, if you
// don't care about ordering.
void remove_and_replace_with_back(ListData** data, int index);

// Print out the contents of the list, with syntax [value1,value2,value3]
std::string to_string(ListData* value);

} // namespace tvvector
} // namespace circa
