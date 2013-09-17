// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

bool is_hashtable(caValue* value);
void set_hashtable(caValue* value);
void set_mutable_hashtable(caValue* value);
caValue* hashtable_get(caValue* table, caValue* key);
caValue* hashtable_get(caValue* table, const char* keyStr);
caValue* hashtable_insert(caValue* table, caValue* key, bool consumeKey);
caValue* hashtable_insert(caValue* table, caValue* key);
void hashtable_remove(caValue* table, caValue* key);
bool hashtable_is_empty(caValue* table);
void hashtable_get_keys(caValue* table, caValue* keysOut);

void hashtable_setup_type(Type* type);

} // namespace circa
