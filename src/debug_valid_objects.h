// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "build_options.h"

#if ENABLE_VALID_OBJECT_CHECKING

void debug_register_valid_object(void* obj, int type);
void debug_unregister_valid_object(void* obj);
void debug_assert_valid_object(void* obj, int type);

const int TERM_OBJECT = 1;
const int LIST_OBJECT = 2;

#else

// No-op these calls
#define debug_register_valid_object(...)
#define debug_unregister_valid_object(...)
#define debug_assert_valid_object(...)

#endif
