// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"
#include "build_options.h"

namespace circa {

const int NO_TYPE =               0x0;
const int TERM_OBJECT =           0x1;
const int LIST_OBJECT =           0x2;
const int BRANCH_OBJECT =         0x4;
const int FUNCTION_ATTRS_OBJECT = 0x8;
const int TYPE_OBJECT =           0x10;
const int TAGGED_VALUE_OBJECT =   0x20;

struct HeapEntry
{
    // 'type' is a bitmask of types.
    int type;

    HeapEntry() : type(0) {}
};

#if CIRCA_ENABLE_HEAP_DEBUGGING

void debug_register_valid_object(void* obj, int type);
void debug_unregister_valid_object(void* obj, int type);
void debug_assert_valid_object(void* obj, int type);

// When added to a class, HeapTracker can make registration easier, and it can
// also get around problems caused by static initialization order. If used, it
// must be the first member in the class.
struct HeapTracker
{
    int _type;
    HeapTracker(int type);
    ~HeapTracker();
};

#else

struct HeapTracker
{
    HeapTracker(int) {}
};

// No-op these calls
#define debug_register_valid_object(...)
#define debug_unregister_valid_object(...)
#define debug_assert_valid_object(...)

#endif

// Returns whether this object is valid and is registered with the given
// type. If possible, you are encouraged to use debug_assert_valid_object
// instead of this call.
bool debug_is_object_valid(void* obj, int type);

// Type-specific calls below:

// Checks if term is a valid pointer according to our map, and triggers
// an ca_assert if not.
void assert_valid_term(Term* term);

}
