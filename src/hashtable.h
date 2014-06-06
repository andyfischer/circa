// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void set_mutable_hashtable(caValue* value);
void hashtable_touch(caValue* value);
bool hashtable_touch_is_necessary(caValue* value);

caValue* hashtable_get(caValue* table, caValue* key);
caValue* hashtable_get(caValue* table, const char* keyStr);
caValue* hashtable_insert(caValue* table, caValue* key, bool moveKey);
caValue* hashtable_insert(caValue* table, caValue* key);
void hashtable_remove(caValue* table, caValue* key);

caValue* hashtable_get_int_key(caValue* table, int key);
caValue* hashtable_insert_int_key(caValue* table, int key);
void hashtable_remove_int_key(caValue* table, int key);

caValue* hashtable_get_symbol_key(caValue* table, Symbol key);
caValue* hashtable_insert_symbol_key(caValue* table, Symbol key);
void hashtable_remove_symbol_key(caValue* table, Symbol key);

caValue* hashtable_insert_string(caValue* table, const char* str);
caValue* hashtable_get_string(caValue* table, const char* str);

bool hashtable_is_empty(caValue* table);
void hashtable_get_keys(caValue* table, caValue* keysOut);

int hashtable_count(caValue* table);
int hashtable_slot_count(caValue* table);
caValue* hashtable_key_by_index(caValue* table, int index);
caValue* hashtable_value_by_index(caValue* table, int index);

void hashtable_setup_type(Type* type);

struct HashtableIterator
{
    caValue* table;
    int index;

    HashtableIterator(caValue* table);

    caValue* currentKey();
    caValue* current();
    void advance();
    bool finished();

    caValue* key() { return currentKey(); }
    caValue* value() { return current(); }

    operator bool() { return !finished(); }
    void operator++() { advance(); }

    void _advanceWhileInvalid();
};

} // namespace circa
