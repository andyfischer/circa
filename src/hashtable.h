// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void set_mutable_hashtable(Value* value);
void hashtable_touch(Value* value);
bool hashtable_touch_is_necessary(Value* value);

Value* hashtable_get(Value* table, Value* key);
Value* hashtable_get(Value* table, const char* keyStr);
Value* hashtable_insert(Value* table, Value* key, bool moveKey);
Value* hashtable_insert(Value* table, Value* key);
void hashtable_remove(Value* table, Value* key);

Value* hashtable_get_int_key(Value* table, int key);
Value* hashtable_insert_int_key(Value* table, int key);
void hashtable_remove_int_key(Value* table, int key);

Value* hashtable_insert_term_key(Value* table, Term* term);
Value* hashtable_get_term_key(Value* table, Term* term);

Value* hashtable_get_symbol_key(Value* table, Symbol key);
Value* hashtable_insert_symbol_key(Value* table, Symbol key);
void hashtable_remove_symbol_key(Value* table, Symbol key);

Value* hashtable_insert_string(Value* table, const char* str);
Value* hashtable_get_string(Value* table, const char* str);

bool hashtable_is_empty(Value* table);
void hashtable_get_keys(Value* table, Value* keysOut);

int hashtable_count(Value* table);
int hashtable_slot_count(Value* table);
Value* hashtable_key_by_index(Value* table, int index);
Value* hashtable_value_by_index(Value* table, int index);

void hashtable_setup_type(Type* type);

struct HashtableIterator
{
    Value* table;
    int index;

    HashtableIterator(Value* table);

    Value* currentKey();
    Value* current();
    void advance();
    bool finished();

    Value* key() { return currentKey(); }
    Value* value() { return current(); }

    operator bool() { return !finished(); }
    void operator++() { advance(); }

    void _advanceWhileInvalid();
};

} // namespace circa
