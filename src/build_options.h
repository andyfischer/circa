// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

// Enabling this flag will have us keep a map of all of our allocations,
// and we'll enable assertions across the code that check if our
// pointers are pointing to valid, correctly-typed objects.
#define ENABLE_VALID_OBJECT_CHECKING 0

// Disable value sharing in tvvector data, each reference will have a separate
// copy of data.
#define DISABLE_LIST_VALUE_SHARING 1
