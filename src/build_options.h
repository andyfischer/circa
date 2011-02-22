// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. Has a
// performance penalty.
#define CIRCA_VALID_OBJECT_CHECKING 0

// Disable value sharing in tvvector data, each reference will have a separate
// copy of data.
#define CIRCA_DISABLE_LIST_SHARING 1

// Trigger an assert when internal_error is called. If this is off, the alternative is
// that an exception is thrown.
#define CIRCA_ASSERT_ON_ERROR 1

#define CIRCA_THROW_ON_ERROR !CIRCA_ASSERT_ON_ERROR

#define CIRCA_ENABLE_TRAP_ON_VALUE_WRITE 0
