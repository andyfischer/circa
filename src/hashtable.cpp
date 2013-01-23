// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/circa.h"
#include "circa/internal/for_hosted_funcs.h"

#include "hashtable.h"
#include "names.h"

namespace circa {

struct Slot {
    caValue key;
    caValue value;
};

struct Hashtable {
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

Hashtable* grow(Hashtable* data, int new_capacity)
{
    Hashtable* new_data = create_table(new_capacity);

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

std::string to_string(Hashtable* data)
{
    std::stringstream strm;
    strm << "{";
    int count = data == NULL ? 0 : data->capacity;
    bool first = true;
    for (int i=0; i < count; i++) {
        if (is_null(&data->slots[i].key))
            continue;

        if (!first)
            strm << ", ";
        first = false;

        strm << circa::to_string(&data->slots[i].key);
        strm << ": " << circa::to_string(&data->slots[i].value);
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
        free_table((Hashtable*) value->value_data.ptr);
    }
    void copy(Type* type, caValue* source, caValue* dest)
    {
        change_type(dest, type);
        dest->value_data.ptr = duplicate((Hashtable*) source->value_data.ptr);
    }
    std::string to_string(caValue* value)
    {
        return to_string((Hashtable*) value->value_data.ptr);
    }
    caValue* get_field(caValue* value, const char* field)
    {
        Value fieldStr;
        set_string(&fieldStr, field);
        return hashtable_get((Hashtable*) value->value_data.ptr, &fieldStr);
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

caValue* hashtable_get(caValue* table, caValue* key)
{
    ca_assert(is_hashtable(table));
    return hashtable_get((Hashtable*) table->value_data.ptr, key);
}

caValue* hashtable_insert(caValue* tableTv, caValue* key, bool consumeKey)
{
    ca_assert(is_hashtable(tableTv));
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    int index = hashtable_insert(&table, key, consumeKey);

    caValue* slot = &table->slots[index].value;
    return slot;
}

caValue* hashtable_insert(caValue* table, caValue* key)
{
    return hashtable_insert(table, key, false);
}

void hashtable_remove(caValue* tableTv, caValue* key)
{
    ca_assert(is_hashtable(tableTv));
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    remove(table, key);
}

void hashtable_setup_type(Type* type)
{
    set_string(&type->name, "Map");
    type->initialize = tagged_value_wrappers::initialize;
    type->release = tagged_value_wrappers::release;
    type->copy = tagged_value_wrappers::copy;
    type->toString = tagged_value_wrappers::to_string;
    type->getField = tagged_value_wrappers::get_field;
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
