// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. This mode
// is fairly slow, but it's good for catching bugs.
#define ENABLE_VALID_OBJECT_CHECKING 0

// Disable value sharing in tvvector data, each reference will have a separate
// copy of data.
#define DISABLE_LIST_VALUE_SHARING 1

// Do frequent sanity checks of code data before executing it. Has a performance
// penalty.
#define AGGRESSIVELY_CHECK_BRANCH_INVARIANTS 1
