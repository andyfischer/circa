// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {
namespace hashtable_t {

struct Hashtable;

int table_insert(Hashtable** dataPtr, TValue* key, bool swapKey);
int find_key(Hashtable* data, TValue* key);
TValue* get_value(Hashtable* data, TValue* key);

void iterator_start(Hashtable* data, TValue* iterator);
void iterator_next(Hashtable* data, TValue* iterator);

bool is_hashtable(TValue* value);
TValue* get_value(TValue* table, TValue* key);
void table_insert(TValue* table, TValue* key, TValue* value,
        bool swapKey, bool swapTValue);
void table_remove(TValue* tableTv, TValue* key);

void setup_type(Type* type);

} // namespace hashtable_t
} // namespace circa
