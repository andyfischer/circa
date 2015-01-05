// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include "circa/circa.h"

#include "hashtable.h"
#include "kernel.h"
#include "list.h"
#include "names.h"
#include "string_type.h"
#include "symbols.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

struct Slot {
    u32 hash;
    int next;
    Value key;
    Value value;
};

struct Hashtable {
    int refCount;
    bool mut;
    int capacity;
    int count;
    int buckets[0];
    // buckets has size [capacity].
    // after buckets is Slot[capacity]
};

int hashtable_insert(Hashtable** dataPtr, Value* key, bool moveKey);
int hashtable_find_key_index(Hashtable* table, Value* key, u32 keyHash);
Value* hashtable_get(Hashtable* table, Value* key);

// How many slots to create for a brand new table.
const int INITIAL_SIZE = 8;

// When reallocating a table, how many slots should initially be filled.
const float INITIAL_LOAD_FACTOR = 0.3f;

// The load at which we'll trigger a reallocation.
const float MAX_LOAD_FACTOR = 0.75f;

static Slot* get_slot(Hashtable* table, int index)
{
    ca_assert(index < table->capacity);
    Slot* firstSlot = (Slot*) &table->buckets[table->capacity];
    return &firstSlot[index];
}

Hashtable* create_table(int capacity)
{
    ca_assert(capacity > 0);
    size_t size = sizeof(Hashtable) + capacity * (sizeof(int) + sizeof(Slot));
    Hashtable* table = (Hashtable*) malloc(size);
    memset(table, 0, size);
    table->refCount = 1;
    table->mut = false;
    table->capacity = capacity;
    for (int i=0; i < capacity; i++) {
        table->buckets[i] = -1;
        Slot* slot = get_slot(table, i);
        initialize_null(&slot->key);
        initialize_null(&slot->value);
    }
    return table;
}

Hashtable* create_table()
{
    return create_table(INITIAL_SIZE);
}

void free_table(Hashtable* table)
{
    if (table == NULL)
        return;

    for (int i=0; i < table->count; i++) {
        Slot* slot = get_slot(table, i);
        set_null(&slot->key);
        set_null(&slot->value);
    }
    free(table);
}

void hashtable_decref(Hashtable* table)
{
    ca_assert(table->refCount > 0);
    table->refCount--;

    if (table->refCount == 0)
        free_table(table);
}

void hashtable_incref(Hashtable* table)
{
    ca_assert(table->refCount > 0);
    table->refCount++;
}

static Hashtable* grow(Hashtable* table)
{
    int newCapacity = INITIAL_SIZE;
    int existingCount = 0;
    if (table != NULL) {
        newCapacity = table->count / INITIAL_LOAD_FACTOR;
        existingCount = table->count;
    }

    Hashtable* newTable = create_table(newCapacity);
    newTable->refCount = table->refCount;
    newTable->mut = table->mut;

    // Move all existing keys & values over.
    for (int i=0; i < existingCount; i++) {
        Slot* oldSlot = get_slot(table, i);
        int index = hashtable_insert(&newTable, &oldSlot->key, true);
        Slot* newSlot = get_slot(newTable, index);
        move(&oldSlot->value, &newSlot->value);
    }

    free(table);
    return newTable;
}

Hashtable* duplicate(Hashtable* original)
{
    if (original == NULL)
        return NULL;

    stat_increment(HashtableDuplicate);

    int new_capacity = int(original->count / INITIAL_LOAD_FACTOR);
    if (new_capacity < INITIAL_SIZE)
        new_capacity = INITIAL_SIZE;

    Hashtable* dupe = create_table(new_capacity);

    dupe->mut = original->mut;

    // Copy all items
    for (int i=0; i < original->count; i++) {
        Slot* slot = get_slot(original, i);
        int index = hashtable_insert(&dupe, &slot->key, false);
        Slot* dupeSlot = get_slot(dupe, index);
        copy(&slot->value, &dupeSlot->value);
        stat_increment(HashtableDuplicate_Copy);
    }
    return dupe;
}

void hashtable_copy(Value* sourceVal, Value* destVal)
{
    Hashtable* source = (Hashtable*) sourceVal->value_data.ptr;

    make_no_initialize(sourceVal->value_type, destVal);

    if (source != NULL)
        hashtable_incref(source);

    destVal->value_data.ptr = source;
}

void hashtable_touch(Value* value)
{
    Hashtable* table = (Hashtable*) value->value_data.ptr;
    if (table == NULL || table->mut || table->refCount == 1)
        return;

    Hashtable* copy = duplicate(table);
    hashtable_decref(table);
    value->value_data.ptr = copy;
}

bool hashtable_touch_is_necessary(Value* value)
{
    Hashtable* table = (Hashtable*) value->value_data.ptr;
    return !(table == NULL || table->mut || table->refCount == 1);
}

int hashtable_find_key_index(Hashtable* table, Value* key, u32 keyHash)
{
    if (table == NULL)
        return -1;

    int bucket = keyHash % table->capacity;

    int slotIndex = table->buckets[bucket];

    while (true) {
        if (slotIndex == -1)
            return -1;

        Slot* slot = get_slot(table, slotIndex);
        if (keyHash == slot->hash && strict_equals(key, &slot->key))
            return slotIndex;

        slotIndex = slot->next;
    }
    
    return -1; // Unreachable
}

// Insert the given key into the dictionary, returns the index.
// This may create a new Hashtable* object, so don't use the old Hashtable* pointer after
// calling this.
int hashtable_insert(Hashtable** dataPtr, Value* key, bool moveKey)
{
    if (*dataPtr == NULL)
        *dataPtr = create_table();

    u32 hash = get_hash_value(key);

    // Check if this key is already here.
    int existing = hashtable_find_key_index(*dataPtr, key, hash);
    if (existing != -1)
        return existing;

    // Check if it is time to reallocate
    if ((*dataPtr)->count >= MAX_LOAD_FACTOR * ((*dataPtr)->capacity))
        *dataPtr = grow(*dataPtr);

    Hashtable* table = *dataPtr;

    // Allocate slot
    int newSlotIndex = table->count++;
    Slot* slot = get_slot(table, newSlotIndex);
    slot->next = -1;
    slot->hash = hash;

    if (moveKey)
        move(key, &slot->key);
    else
        copy(key, &slot->key);

    // Add slot to bucket list
    int bucket = hash % table->capacity;

    if (table->buckets[bucket] == -1) {
        // First key in bucket.
        table->buckets[bucket] = newSlotIndex;
    } else {
        Slot* searchSlot = get_slot(table, table->buckets[bucket]);
        while (searchSlot->next != -1)
            searchSlot = get_slot(table, searchSlot->next);

        searchSlot->next = newSlotIndex;
    }

    return newSlotIndex;
}

void insert_value(Hashtable** dataPtr, Value* key, Value* value)
{
    int index = hashtable_insert(dataPtr, key, false);
    Slot* slot = get_slot(*dataPtr, index);
    copy(value, &slot->value);
}

Value* hashtable_get(Hashtable* table, Value* key)
{
    int index = hashtable_find_key_index(table, key, get_hash_value(key));
    if (index == -1)
        return NULL;
    Slot* slot = get_slot(table, index);
    return &slot->value;
}

Value* get_index(Hashtable* data, int index)
{
    ca_assert(index < data->capacity);
    Slot* slot = get_slot(data, index);
    return &slot->value;
}

void move_slot(Hashtable* table, int fromIndex, int toIndex)
{
    if (fromIndex == toIndex)
        return;

    Slot* from = get_slot(table, fromIndex);
    Slot* to = get_slot(table, toIndex);

    move(&from->key, &to->key);
    move(&from->value, &to->value);
    to->hash = from->hash;
    to->next = from->next;

    // Linear search: find and update whathever was pointing to 'from'
    for (int i=0; i < table->capacity; i++) {
        if (table->buckets[i] == fromIndex) {
            table->buckets[i] = toIndex;
            return;
        }
    }
    for (int i=0; i < table->count; i++) {
        if (get_slot(table, i)->next == fromIndex) {
            get_slot(table, i)->next = toIndex;
            return;
        }
    }
}

void remove(Hashtable* table, Value* key)
{
    if (table == NULL)
        return;

    u32 hash = get_hash_value(key);
    int bucket = hash % table->capacity;

    Slot* previousSlot = NULL;
    int searchIndex = table->buckets[bucket];

    while (true) {
        if (searchIndex == -1)
            return;

        Slot* searchSlot = get_slot(table, searchIndex);

        if (searchSlot->hash == hash && strict_equals(&searchSlot->key, key)) {
            // Found key.

            set_null(&searchSlot->key);
            set_null(&searchSlot->value);

            if (previousSlot == NULL)
                table->buckets[bucket] = searchSlot->next;
            else
                previousSlot->next = searchSlot->next;

            // Fill in the open slot
            table->count--;
            move_slot(table, table->count, searchIndex);

            return;
        }

        previousSlot = searchSlot;
        searchIndex = searchSlot->next;
    }
}

bool is_empty(Hashtable* data)
{
    if (data == NULL)
        return true;

    return data->count == 0;
}

int count(Hashtable* data)
{
    return data->count;
}

void clear(Hashtable* table)
{
    for (int i=0; i < table->count; i++) {
        Slot* slot = get_slot(table, i);
        set_null(&slot->key);
        set_null(&slot->value);
    }
    table->count = 0;
}

static void hashtable_to_string(Value* table, Value* out)
{
    if (table == NULL || hashtable_count(table) == 0) {
        string_append(out, "{}");
        return;
    }

    Value keys;
    hashtable_get_keys(table, &keys);
    list_sort(&keys, NULL, NULL);

    string_append(out, "{");

    for (int i=0; i < list_length(&keys); i++) {
        if (i > 0)
            string_append(out, ", ");

        Value* key = list_get(&keys, i);

        Value* value = hashtable_get(table, key);

        Value keyAsString;
        to_string(key, &keyAsString);
        string_append(out, &keyAsString);

        Value valueAsString;
        to_string(value, &valueAsString);

        string_append(out, " => ");
        string_append(out, &valueAsString);
    }

    string_append(out, "}");
}

namespace tagged_value_wrappers {

    void initialize(Type* type, Value* value)
    {
        value->value_data.ptr = NULL;
    }
    void release(Value* value)
    {
        Hashtable* table = (Hashtable*) value->value_data.ptr;
        if (table == NULL)
            return;

        hashtable_decref(table);
    }
} // namespace tagged_value_wrappers

void set_mutable_hashtable(Value* value)
{
    make_no_initialize(TYPES.map, value);
    Hashtable* table = create_table();
    table->mut = true;
    value->value_data.ptr = table;
}

Value* hashtable_get(Value* table, Value* key)
{
    ca_assert(table->value_type->storageType == s_StorageTypeHashtable);
    return hashtable_get((Hashtable*) table->value_data.ptr, key);
}

Value* hashtable_get(Value* table, const char* keystr)
{
    Value str;
    set_string(&str, keystr);
    return hashtable_get(table, &str);
}

Value* hashtable_insert(Value* tableTv, Value* key, bool moveKey)
{
    ca_assert(is_hashtable(tableTv));
    hashtable_touch(tableTv);
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    int index = hashtable_insert(&table, key, moveKey);

    return &get_slot(table, index)->value;
}

Value* hashtable_insert(Value* table, Value* key)
{
    return hashtable_insert(table, key, false);
}

Value* hashtable_get_int_key(Value* table, int key)
{
    // Future: Optimize by not creating a Value.
    Value boxedKey;
    set_int(&boxedKey, key);
    return hashtable_get(table, &boxedKey);
}

Value* hashtable_get_term_key(Value* table, Term* term)
{
    Value boxedKey;
    set_term_ref(&boxedKey, term);
    return hashtable_get(table, &boxedKey);
}

Value* hashtable_insert_int_key(Value* table, int key)
{
    Value boxedKey;
    set_int(&boxedKey, key);
    return hashtable_insert(table, &boxedKey);
}

Value* hashtable_get_symbol_key(Value* table, Symbol key)
{
    Value boxedKey;
    set_symbol(&boxedKey, key);
    return hashtable_get(table, &boxedKey);
}

Value* hashtable_insert_symbol_key(Value* table, Symbol key)
{
    Value boxedKey;
    set_symbol(&boxedKey, key);
    return hashtable_insert(table, &boxedKey);
}

void hashtable_remove_symbol_key(Value* table, Symbol key)
{
    Value boxedKey;
    set_symbol(&boxedKey, key);
    hashtable_remove(table, &boxedKey);
}

void hashtable_remove_int_key(Value* table, int key)
{
    Value boxedKey;
    set_int(&boxedKey, key);
    hashtable_remove(table, &boxedKey);
}

void hashtable_remove(Value* tableTv, Value* key)
{
    ca_assert(is_hashtable(tableTv));
    hashtable_touch(tableTv);
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    remove(table, key);
}

bool hashtable_is_empty(Value* value)
{
    ca_assert(is_hashtable(value));
    Hashtable* table = (Hashtable*) value->value_data.ptr;
    return is_empty(table);
}

void hashtable_get_keys(Value* tableVal, Value* keysOut)
{
    set_list(keysOut);

    ca_assert(is_hashtable(tableVal));
    Hashtable* table = (Hashtable*) tableVal->value_data.ptr;
    if (table == NULL)
        return;

    for (int i=0; i < table->count; i++) {
        Slot* slot = get_slot(table, i);
        copy(&slot->key, list_append(keysOut));
    }
}

int hashtable_count(Value* tableVal)
{
    ca_assert(is_hashtable(tableVal));
    Hashtable* table = (Hashtable*) tableVal->value_data.ptr;
    if (table == NULL)
        return 0;
    return table->count;
}

int hashtable_slot_count(Value* tableVal)
{
    ca_assert(is_hashtable(tableVal));
    if (hashtable_is_empty(tableVal))
        return 0;
    return ((Hashtable*) tableVal->value_data.ptr)->capacity;
}
Value* hashtable_key_by_index(Value* tableVal, int index)
{
    ca_assert(is_hashtable(tableVal));
    Hashtable* table = (Hashtable*) tableVal->value_data.ptr;
    return &get_slot(table, index)->key;
}
Value* hashtable_value_by_index(Value* tableVal, int index)
{
    ca_assert(is_hashtable(tableVal));
    Hashtable* table = (Hashtable*) tableVal->value_data.ptr;
    return &get_slot(table, index)->value;
}

bool hashtable_equals(Value* left, Value* right)
{
    if (!is_hashtable(right))
        return false;
    if (hashtable_count(left) != hashtable_count(right))
        return false;

    Hashtable* leftTable = (Hashtable*) left->value_data.ptr;

    if (leftTable == NULL)
        return true; // already verified that counts are equal

    for (int i=0; i < leftTable->count; i++) {
        Slot* leftSlot = get_slot(leftTable, i);
        Value* rightValue = hashtable_get(right, &leftSlot->key);
        if (rightValue == NULL)
            return false;
        if (!equals(&leftSlot->value, rightValue))
            return false;
    }
    return true;
}

u32 circular_shift(u32 value, int shift)
{
    shift = shift % 32;
    if (shift == 0)
        return value;
    else
        return (value << shift) | (value >> (32 - shift));
}

int hashtable_hash(Value* value)
{
    int hash = 0;
    Hashtable* table = (Hashtable*) value->value_data.ptr;
    if (table == NULL)
        return 0;

    for (int i=0; i < table->count; i++) {
        Slot* slot = get_slot(table, i);
        int itemHash = get_hash_value(&slot->key);
        itemHash ^= circular_shift(get_hash_value(&slot->value), 16);
        hash ^= circular_shift(itemHash, i);
    }
    return hash;
}

void hashtable_setup_type(Type* type)
{
    set_string(&type->name, "Map");
    type->initialize = tagged_value_wrappers::initialize;
    type->release = tagged_value_wrappers::release;
    type->copy = hashtable_copy;
    type->equals = hashtable_equals;
    type->hashFunc = hashtable_hash;
    type->toString = hashtable_to_string;
    type->storageType = s_StorageTypeHashtable;
}

HashtableIterator::HashtableIterator(Value* _table)
  : table(_table), index(0)
{
    _advanceWhileInvalid();
}

Value* HashtableIterator::currentKey()
{
    return hashtable_key_by_index(table, index);
}

Value* HashtableIterator::current()
{
    return hashtable_value_by_index(table, index);
}

void HashtableIterator::advance()
{
    index++;
    _advanceWhileInvalid();
}

bool HashtableIterator::finished()
{
    return index >= hashtable_count(table);
}

void HashtableIterator::_advanceWhileInvalid()
{
}

CIRCA_EXPORT Value* circa_map_insert(Value* map, Value* key)
{
    return hashtable_insert(map, key, false);
}

CIRCA_EXPORT Value* circa_map_get(Value* map, Value* key)
{
    return hashtable_get(map, key);
}

CIRCA_EXPORT Value* circa_map_insert_move(Value* map, Value* key)
{
    return hashtable_insert(map, key, true);
}

CIRCA_EXPORT void circa_map_set_int(Value* map, Value* key, int val)
{
    Value wrapped;
    set_int(&wrapped, val);
    move(&wrapped, circa_map_insert(map, key));
}

CIRCA_EXPORT void circa_set_map(Value* value)
{
    set_hashtable(value);
}

} // namespace circa
