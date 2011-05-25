// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

void recursive_dump_heap(TaggedValue* value, const char* prefix);
int count_references_to_pointer(TaggedValue* container, void* ptr);

} // namespace circa
