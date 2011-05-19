// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {
namespace hashtable_t {

struct Hashtable;

int table_insert(Hashtable** dataPtr, Value* key, bool swapKey);
int find_key(Hashtable* data, Value* key);
Value* get_value(Hashtable* data, Value* key);

void iterator_start(Hashtable* data, Value* iterator);
void iterator_next(Hashtable* data, Value* iterator);

bool is_hashtable(Value* value);
Value* get_value(Value* table, Value* key);
void table_insert(Value* table, Value* key, Value* value,
        bool swapKey, bool swapValue);
void table_remove(Value* tableTv, Value* key);

void setup_type(Type* type);

} // namespace hashtable_t
} // namespace circa
