// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

// CIRCA_ENABLE_HEAP_DEBUGGING
//
// Enabling this flag will have us keep a map of all of our allocations
// (for types that support it). We'll also turn on a bunch of assert calls
// which use this map to check if their object pointers are valid. Has a
// large performance penalty.

#ifdef CIRCA_TEST_BUILD

#define CIRCA_ENABLE_HEAP_DEBUGGING 0

#else

#define CIRCA_ENABLE_HEAP_DEBUGGING 0

#endif

// Trigger an assert when internal_error is called. If this is off, the alternative is
// that an exception is thrown.
#define CIRCA_ASSERT_ON_ERROR 1

#define CIRCA_THROW_ON_ERROR !CIRCA_ASSERT_ON_ERROR

#define CIRCA_ENABLE_TAGGED_VALUE_METADATA 0

