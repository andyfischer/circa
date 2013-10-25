// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"
#include "circa/internal/for_hosted_funcs.h"

#include <algorithm>

#include "hashtable.h"
#include "names.h"

namespace circa {

struct Slot {
    Value key;
    Value value;
};

struct Hashtable {
    int refCount;
    bool mut;
    int capacity;
    int count;
    Slot slots[0];
    // slots has size [capacity].
};

int hashtable_insert(Hashtable** dataPtr, caValue* key, bool consumeKey);
int hashtable_find_slot(Hashtable* data, caValue* key);
caValue* hashtable_get(Hashtable* data, caValue* key);

// How many slots to create for a brand new table.
const int INITIAL_SIZE = 10;

// When reallocating a table, how many slots should initially be filled.
const float INITIAL_LOAD_FACTOR = 0.3f;

// The load at which we'll trigger a reallocation.
const float MAX_LOAD_FACTOR = 0.75f;

Hashtable* create_table(int capacity)
{
    ca_assert(capacity > 0);
    Hashtable* result = (Hashtable*) malloc(sizeof(Hashtable) + capacity * sizeof(Slot));
    result->refCount = 1;
    result->mut = false;
    result->capacity = capacity;
    result->count = 0;
    memset(result->slots, 0, capacity * sizeof(Slot));
    for (int s=0; s < capacity; s++) {
        initialize_null(&result->slots[s].key);
        initialize_null(&result->slots[s].value);
    }
    return result;
}

Hashtable* create_table()
{
    return create_table(INITIAL_SIZE);
}

void free_table(Hashtable* data)
{
    if (data == NULL)
        return;

    for (int i=0; i < data->capacity; i++) {
        set_null(&data->slots[i].key);
        set_null(&data->slots[i].value);
    }
    free(data);
}

void hashtable_decref(Hashtable* data)
{
    ca_assert(data->refCount > 0);
    data->refCount--;

    if (data->refCount == 0)
        free_table(data);
}

void hashtable_incref(Hashtable* data)
{
    ca_assert(data->refCount > 0);
    data->refCount++;
}

Hashtable* grow(Hashtable* data, int new_capacity)
{
    Hashtable* new_data = create_table(new_capacity);
    new_data->mut = data->mut;

    int existingCapacity = 0;
    if (data != NULL)
        existingCapacity = data->capacity;

    // Move all the keys & values over.
    for (int i=0; i < existingCapacity; i++) {
        Slot* old_slot = &data->slots[i];

        if (is_null(&old_slot->key))
            continue;

        int index = hashtable_insert(&new_data, &old_slot->key, true);
        swap(&old_slot->value, &new_data->slots[index].value);
    }
    return new_data;
}

// Grow this dictionary by the default growth rate. This will result in a new Hashtable*
// object, don't use the old one after calling this.
void grow(Hashtable** dataPtr)
{
    int new_capacity = int((*dataPtr)->count / INITIAL_LOAD_FACTOR);
    Hashtable* oldData = *dataPtr;
    *dataPtr = grow(*dataPtr, new_capacity);
    free_table(oldData);
}

Hashtable* duplicate(Hashtable* original)
{
    if (original == NULL)
        return NULL;

    int new_capacity = int(original->count / INITIAL_LOAD_FACTOR);
    if (new_capacity < INITIAL_SIZE)
        new_capacity = INITIAL_SIZE;

    Hashtable* dupe = create_table(new_capacity);

    dupe->mut = original->mut;

    // Copy all items
    for (int i=0; i < original->capacity; i++) {
        Slot* slot = &original->slots[i];

        if (is_null(&slot->key))
            continue;

        int index = hashtable_insert(&dupe, &slot->key, false);
        copy(&slot->value, &dupe->slots[index].value);
    }
    return dupe;
}

void hashtable_copy(Type*, caValue* sourceVal, caValue* destVal)
{
    Hashtable* source = (Hashtable*) sourceVal->value_data.ptr;

    make_no_initialize(sourceVal->value_type, destVal);

    if (source != NULL)
        hashtable_incref(source);

    destVal->value_data.ptr = source;
}

void hashtable_touch(caValue* value)
{
    Hashtable* table = (Hashtable*) value->value_data.ptr;
    if (table == NULL || table->mut || table->refCount == 1)
        return;

    Hashtable* copy = duplicate(table);
    hashtable_decref(table);
    value->value_data.ptr = copy;
}

// Get the 'ideal' slot index, the place we would put this key if there is no
// collision.
int find_ideal_slot_index(Hashtable* data, caValue* key)
{
    ca_assert(data->capacity > 0);
    unsigned int hash = get_hash_value(key);
    return int(hash % data->capacity);
}

// Insert the given key into the dictionary, returns the index.
// This may create a new Hashtable* object, so don't use the old Hashtable* pointer after
// calling this.
int hashtable_insert(Hashtable** dataPtr, caValue* key, bool consumeKey)
{
    if (*dataPtr == NULL)
        *dataPtr = create_table();

    // Check if this key is already here
    int existing = hashtable_find_slot(*dataPtr, key);
    if (existing != -1)
        return existing;

    // Check if it is time to reallocate
    if ((*dataPtr)->count >= MAX_LOAD_FACTOR * ((*dataPtr)->capacity))
        grow(dataPtr);

    Hashtable* data = *dataPtr;
    int index = find_ideal_slot_index(data, key);

    // Linearly advance if this slot is being used
    int starting_index = index;
    while (!is_null(&data->slots[index].key)) {
        index = (index + 1) % data->capacity;

        // Die if we made it all the way around
        if (index == starting_index) {
            ca_assert(false);
            return 0;
        }
    }

    Slot* slot = &data->slots[index];

    if (consumeKey)
        swap(key, &slot->key);
    else
        copy(key, &slot->key);

    data->count++;

    return index;
}

void insert_value(Hashtable** dataPtr, caValue* key, caValue* value)
{
    int index = hashtable_insert(dataPtr, key, false);
    copy(value, &(*dataPtr)->slots[index].value);
}

int hashtable_find_slot(Hashtable* data, caValue* key)
{
    if (data == NULL)
        return -1;

    int index = find_ideal_slot_index(data, key);
    int starting_index = index;
    while (!equals(key, &data->slots[index].key)) {
        index = (index + 1) % data->capacity;

        // If we hit an empty slot, then give up.
        if (is_null(&data->slots[index].key))
            return -1;

        // Or, if we looped around to the starting index, give up.
        if (index == starting_index)
            return -1;
    }

    return index;
}

caValue* hashtable_get(Hashtable* data, caValue* key)
{
    int index = hashtable_find_slot(data, key);
    if (index == -1) return NULL;
    return &data->slots[index].value;
}


caValue* get_index(Hashtable* data, int index)
{
    ca_assert(index < data->capacity);
    return &data->slots[index].value;
}

void remove(Hashtable* data, caValue* key)
{
    int index = hashtable_find_slot(data, key);
    if (index == -1)
        return;

    // Clear out this slot
    set_null(&data->slots[index].key);
    set_null(&data->slots[index].value);
    data->count--;

    // Check if any following keys would prefer to be moved up to this empty slot.
    while (true) {
        int prevIndex = index;
        index = (index+1) % data->capacity;

        Slot* slot = &data->slots[index];

        if (is_null(&slot->key))
            break;

        // If a slot isn't in its ideal index, then we assume that it would rather be in
        // this slot.
        if (find_ideal_slot_index(data, &slot->key) != index) {
            Slot* prevSlot = &data->slots[prevIndex];
            move(&slot->key, &prevSlot->key);
            move(&slot->value, &prevSlot->value);
            continue;
        }

        break;
    }
}

bool is_empty(Hashtable* data)
{
    if (data == NULL)
        return true;

#ifdef DEBUG
    if (data->count == 0) {
        for (int i=0; i < data->capacity; i++) {
            ca_assert(is_null(&data->slots[i].key));
        }
    }
#endif

    return data->count == 0;
}

int count(Hashtable* data)
{
    return data->count;
}

void clear(Hashtable* data)
{
    for (int i=0; i < data->capacity; i++) {
        Slot* slot = &data->slots[i];
        if (is_null(&slot->key))
            continue;
        set_null(&slot->key);
        set_null(&slot->value);
    }
    data->count = 0;
}

struct HashtableToStringEntry {
    caValue* key;
    int valueIndex;

    bool operator<(const HashtableToStringEntry& rhs) const {
        return string_less_than(key, rhs.key);
    }
};

std::string to_string(Hashtable* data)
{
    if (data == NULL || data->capacity == 0)
        return "{}";

    std::stringstream strm;

    int count = data->capacity;
    Value keyStrings;
    set_list(&keyStrings, count);

    for (int i=0; i < count; i++) {
        caValue* originalKey = &data->slots[i].key;
        if (is_null(originalKey))
            continue;
        
        caValue* str = list_get(&keyStrings, i);

        if (is_string(originalKey))
            copy(originalKey, str);
        else
            set_string(str, circa::to_string(originalKey));
    }

    // Short-term solution: use std library for sorting.
    std::vector<HashtableToStringEntry> entryList;

    for (int i=0; i < count; i++) {
        caValue* str = list_get(&keyStrings, i);
        if (is_null(str))
            continue;

        HashtableToStringEntry entry;
        entry.key = str;
        entry.valueIndex = i;
        entryList.push_back(entry);
    }

    std::sort(entryList.begin(), entryList.end());

    strm << "{";

    bool first = true;
    for (int i=0; i < (int) entryList.size(); i++) {
        caValue* key = entryList[i].key;

        if (is_null(key))
            continue;

        if (!first)
            strm << ", ";
        first = false;

        caValue* value = &data->slots[entryList[i].valueIndex].value;

        strm << circa::as_string(key);
        strm << ": " << circa::to_string(value);
    }
    strm << "}";
    return strm.str();
}

void debug_print(Hashtable* data)
{
    printf("dict: %p\n", data);
    printf("count: %d, capacity: %d\n", data->count, data->capacity);
    for (int i=0; i < data->capacity; i++) {
        printf("[%d] %s = %s\n", i, circa::to_string(&data->slots[i].key).c_str(),
                circa::to_string(&data->slots[i].value).c_str());
    }
}

namespace tagged_value_wrappers {

    void initialize(Type* type, caValue* value)
    {
        value->value_data.ptr = NULL;
    }
    void release(caValue* value)
    {
        Hashtable* table = (Hashtable*) value->value_data.ptr;
        if (table == NULL)
            return;

        hashtable_decref(table);
    }
    std::string to_string(caValue* value)
    {
        return to_string((Hashtable*) value->value_data.ptr);
    }
} // namespace tagged_value_wrappers

// Public API
bool is_hashtable(caValue* value)
{
    return value->value_type->storageType == sym_StorageTypeHashtable;
}

void set_hashtable(caValue* value)
{
    make(TYPES.map, value);
}

void set_mutable_hashtable(caValue* value)
{
    make_no_initialize(TYPES.map, value);
    Hashtable* table = create_table();
    table->mut = true;
    value->value_data.ptr = table;
}

caValue* hashtable_get(caValue* table, caValue* key)
{
    ca_assert(is_hashtable(table));
    return hashtable_get((Hashtable*) table->value_data.ptr, key);
}

caValue* hashtable_get(caValue* table, const char* keystr)
{
    Value str;
    set_string(&str, keystr);
    return hashtable_get(table, &str);
}

caValue* hashtable_insert(caValue* tableTv, caValue* key, bool consumeKey)
{
    hashtable_touch(tableTv);
    ca_assert(is_hashtable(tableTv));
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    int index = hashtable_insert(&table, key, consumeKey);

    caValue* slot = &table->slots[index].value;
    return slot;
}

caValue* hashtable_insert(caValue* table, caValue* key)
{
    hashtable_touch(table);
    return hashtable_insert(table, key, false);
}

void hashtable_remove(caValue* tableTv, caValue* key)
{
    ca_assert(is_hashtable(tableTv));
    hashtable_touch(tableTv);
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    remove(table, key);
}

bool hashtable_is_empty(caValue* value)
{
    ca_assert(is_hashtable(value));
    Hashtable*& table = (Hashtable*&) value->value_data.ptr;
    return is_empty(table);
}

void hashtable_get_keys(caValue* table, caValue* keysOut)
{
    set_list(keysOut);

    ca_assert(is_hashtable(table));
    Hashtable* data = (Hashtable*) table->value_data.ptr;
    if (data == NULL)
        return;

    for (int i=0; i < data->capacity; i++) {
        if (is_null(&data->slots[i].key))
            continue;
        copy(&data->slots[i].key, list_append(keysOut));
    }
}

int hashtable_slot_count(caValue* table)
{
    ca_assert(is_hashtable(table));
    if (hashtable_is_empty(table))
        return 0;
    return ((Hashtable*) table->value_data.ptr)->capacity;
}
caValue* hashtable_key_by_index(caValue* table, int index)
{
    ca_assert(is_hashtable(table));
    return &((Hashtable*) table->value_data.ptr)->slots[index].key;
}
caValue* hashtable_value_by_index(caValue* table, int index)
{
    ca_assert(is_hashtable(table));
    return &((Hashtable*) table->value_data.ptr)->slots[index].value;
}

bool hashtable_equals(caValue* left, caValue* right)
{
    if (!is_hashtable(right))
        return false;
    if (hashtable_is_empty(left))
        return hashtable_is_empty(right);
    if (hashtable_is_empty(right))
        return false;

    Hashtable* leftData = (Hashtable*) left->value_data.ptr;
    Hashtable* rightData = (Hashtable*) right->value_data.ptr;

    if (leftData->count != rightData->count)
        return false;

    for (int i=0; i < leftData->capacity; i++) {
        if (is_null(&leftData->slots[i].key))
            continue;

        caValue* leftKey = &leftData->slots[i].key;
        caValue* leftVal = &leftData->slots[i].value;
        caValue* rightVal = hashtable_get(right, leftKey);

        if (rightVal == NULL || !equals(leftVal, rightVal))
            return false;
    }
    return true;
}

void hashtable_setup_type(Type* type)
{
    set_string(&type->name, "Map");
    type->initialize = tagged_value_wrappers::initialize;
    type->release = tagged_value_wrappers::release;
    type->copy = hashtable_copy;
    type->touch = hashtable_touch;
    type->equals = hashtable_equals;
    type->toString = tagged_value_wrappers::to_string;
    type->storageType = sym_StorageTypeHashtable;
}

// Publich functions
CIRCA_EXPORT caValue* circa_map_insert(caValue* table, caValue* key)
{
    return hashtable_insert(table, key);
}

CIRCA_EXPORT void circa_set_map(caValue* value)
{
    set_hashtable(value);
}

} // namespace circa
