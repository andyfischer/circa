// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {
namespace hashtable_t {

struct Hashtable;

int table_insert(Hashtable** dataPtr, caValue* key, bool consumeKey);
int find_key(Hashtable* data, caValue* key);
caValue* get_value(Hashtable* data, caValue* key);

void iterator_start(Hashtable* data, caValue* iterator);
void iterator_next(Hashtable* data, caValue* iterator);

bool is_hashtable(caValue* value);
caValue* get_value(caValue* table, caValue* key);
caValue* table_insert(caValue* table, caValue* key, bool consumeKey);
void table_remove(caValue* tableTv, caValue* key);

void setup_type(Type* type);

} // namespace hashtable_t
} // namespace circa
