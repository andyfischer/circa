// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. Has a
// performance penalty.
#define ENABLE_VALID_OBJECT_CHECKING 0

// Disable value sharing in tvvector data, each reference will have a separate
// copy of data.
#define DISABLE_LIST_VALUE_SHARING 1

// Enable frequent type checks in situations where they should be unnecessary. Has
// a performance penalty.
#define ENABLE_UNNECESSARY_TYPE_CHECKS 1

// Trigger an assert when internal_error is called. If this is off, the alternative
// is that an exception is thrown.
#define ASSERT_INTERNAL_ERROR 1

#define THROW_INTERNAL_ERROR !ASSERT_INTERNAL_ERROR
