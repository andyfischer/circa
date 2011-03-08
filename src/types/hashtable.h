// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {
namespace hashtable_t {

struct Hashtable;

int table_insert(Hashtable** dataPtr, TaggedValue* key, bool swapKey);
int find_key(Hashtable* data, TaggedValue* key);
TaggedValue* get_value(Hashtable* data, TaggedValue* key);

void iterator_start(Hashtable* data, TaggedValue* iterator);
void iterator_next(Hashtable* data, TaggedValue* iterator);

bool is_hashtable(TaggedValue* value);
TaggedValue* get_value(TaggedValue* table, TaggedValue* key);
void table_insert(TaggedValue* table, TaggedValue* key, TaggedValue* value,
        bool swapKey, bool swapValue);
void table_remove(TaggedValue* tableTv, TaggedValue* key);

void setup_type(Type* type);

} // namespace hashtable_t
} // namespace circa
