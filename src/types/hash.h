// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {
namespace hash_t {

struct HashTable;

int table_insert(HashTable** dataPtr, TaggedValue* key, bool swapKey);
int find_key(HashTable* data, TaggedValue* key);

void iterator_start(HashTable* data, TaggedValue* iterator);
void iterator_next(HashTable* data, TaggedValue* iterator);

} // namespace hash_t
} // namespace circa
