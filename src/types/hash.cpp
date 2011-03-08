// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "hash.h"

namespace circa {
namespace hash_t {

struct Slot {
    TaggedValue key;
    TaggedValue value;
};

struct HashTable {
    int capacity;
    int count;
    Slot slots[0];
    // slots has size [capacity].
};

// How many slots to create for a brand new table.
const int INITIAL_SIZE = 10;

// When reallocating a table, how many slots should initially be filled.
const float INITIAL_LOAD_FACTOR = 0.3;

// The load at which we'll trigger a reallocation.
const float MAX_LOAD_FACTOR = 0.75;

int get_hash_value(TaggedValue* value)
{
    Type::HashFunc f = value->value_type->hashFunc;
    ca_assert(f != NULL);
    return f(value);
}

HashTable* create_table(int capacity)
{
    ca_assert(capacity > 0);
    HashTable* result = (HashTable*) malloc(sizeof(HashTable) + capacity * sizeof(Slot));
    result->capacity = capacity;
    result->count = 0;
    memset(result->slots, 0, capacity * sizeof(Slot));
    for (int s=0; s < capacity; s++) {
        result->slots[s].key.init();
        result->slots[s].value.init();
    }
    return result;
}

HashTable* create_table()
{
    return create_table(INITIAL_SIZE);
}

void free_table(HashTable* data)
{
    if (data == NULL)
        return;

    for (int i=0; i < data->capacity; i++) {
        set_null(&data->slots[i].key);
        set_null(&data->slots[i].value);
    }
    free(data);
}

HashTable* grow(HashTable* data, int new_capacity)
{
    HashTable* new_data = create_table(new_capacity);

    int existingCapacity = 0;
    if (data != NULL)
        existingCapacity = data->capacity;

    // Move all the keys & values over.
    for (int i=0; i < existingCapacity; i++) {
        Slot* old_slot = &data->slots[i];

        if (is_null(&old_slot->key))
            continue;

        int index = table_insert(&new_data, &old_slot->key, true);
        swap(&old_slot->value, &new_data->slots[index].value);
    }
    return new_data;
}

// Grow this dictionary by the default growth rate. This will result in a new HashTable*
// object, don't use the old one after calling this.
void grow(HashTable** dataPtr)
{
    int new_capacity = int((*dataPtr)->count / INITIAL_LOAD_FACTOR);
    HashTable* oldData = *dataPtr;
    *dataPtr = grow(*dataPtr, new_capacity);
    free_table(oldData);
}

HashTable* duplicate(HashTable* original)
{
    if (original == NULL)
        return NULL;

    int new_capacity = int(original->count / INITIAL_LOAD_FACTOR);
    if (new_capacity < INITIAL_SIZE)
        new_capacity = INITIAL_SIZE;

    HashTable* dupe = create_table(new_capacity);

    // Copy all items
    for (int i=0; i < original->capacity; i++) {
        Slot* slot = &original->slots[i];

        if (is_null(&slot->key))
            continue;

        int index = table_insert(&dupe, &slot->key, false);
        copy(&slot->value, &dupe->slots[index].value);
    }
    return dupe;
}

// Get the 'ideal' slot index, the place we would put this key if there is no
// collision.
int find_ideal_slot_index(HashTable* data, TaggedValue* key)
{
    ca_assert(data->capacity > 0);
    unsigned int hash = get_hash_value(key);
    return int(hash % data->capacity);
}

// Insert the given key into the dictionary, returns the index.
// This may create a new HashTable* object, so don't use the old HashTable* pointer after
// calling this.
int table_insert(HashTable** dataPtr, TaggedValue* key, bool swapKey)
{
    if (*dataPtr == NULL)
        *dataPtr = create_table();

    // Check if this key is already here
    int existing = find_key(*dataPtr, key);
    if (existing != -1)
        return existing;

    // Check if it is time to reallocate
    if ((*dataPtr)->count >= MAX_LOAD_FACTOR * ((*dataPtr)->capacity))
        grow(dataPtr);

    HashTable* data = *dataPtr;
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
    swap_or_copy(key, &slot->key, swapKey);
    data->count++;

    return index;
}

void insert_value(HashTable** dataPtr, TaggedValue* key, TaggedValue* value)
{
    int index = table_insert(dataPtr, key, false);
    copy(value, &(*dataPtr)->slots[index].value);
}

int find_key(HashTable* data, TaggedValue* key)
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

TaggedValue* get_value(HashTable* data, TaggedValue* key)
{
    int index = find_key(data, key);
    if (index == -1) return NULL;
    return &data->slots[index].value;
}

TaggedValue* get_index(HashTable* data, int index)
{
    ca_assert(index < data->capacity);
    return &data->slots[index].value;
}

void remove(HashTable* data, TaggedValue* key)
{
    int index = find_key(data, key);
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
            prevSlot->key = slot->key;
            slot->key = NULL;
            swap(&slot->value, &prevSlot->value);
            continue;
        }

        break;
    }
}

int count(HashTable* data)
{
    return data->count;
}

void clear(HashTable* data)
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

std::string to_string(HashTable* data)
{
    std::stringstream strm;
    strm << "[";
    for (int i=0; i < data->capacity; i++) {
        if (is_null(&data->slots[i].key))
            continue;

        strm << to_string(&data->slots[i].key);
        strm << ": " << to_string(&data->slots[i].value);
    }
    strm << "]";
    return strm.str();
}

void debug_print(HashTable* data)
{
    printf("dict: %p\n", data);
    printf("count: %d, capacity: %d\n", data->count, data->capacity);
    for (int i=0; i < data->capacity; i++) {
        printf("[%d] %s = %s\n", i, to_string(&data->slots[i].key).c_str(),
                to_string(&data->slots[i].value).c_str());
    }
}

void iterator_start(HashTable* data, TaggedValue* iterator)
{
    if (data == NULL || data->count == 0)
        return set_null(iterator);

    set_int(iterator, 0);

    // Advance if this iterator location isn't valid
    if (is_null(&data->slots[0].key))
        iterator_next(data, iterator);
}

void iterator_next(HashTable* data, TaggedValue* iterator)
{
    int i = as_int(iterator);

    // Advance to next valid location
    int next = i + 1;
    while ((next < data->capacity) && (is_null(&data->slots[next].key)))
        next++;

    if (next >= data->capacity)
        set_null(iterator);
    else
        set_int(iterator, next);
}

void iterator_get(HashTable* data, TaggedValue* iterator, TaggedValue** key, TaggedValue** value)
{
    int i = as_int(iterator);

    *key = &data->slots[i].key;
    *value = &data->slots[i].value;
}

namespace tagged_value_wrappers {

    void initialize(Type* type, TaggedValue* value)
    {
        value->value_data.ptr = NULL;
    }
    void release(TaggedValue* value)
    {
        free_table((HashTable*) value->value_data.ptr);
    }
    void copy(TaggedValue* source, TaggedValue* dest)
    {
        release(dest);
        dest->value_data.ptr = duplicate((HashTable*) source->value_data.ptr);
    }
    std::string to_string(TaggedValue* value)
    {
        return to_string((HashTable*) value->value_data.ptr);
    }
    TaggedValue* get_field(TaggedValue* value, const char* field)
    {
        TaggedValue fieldStr;
        set_string(&fieldStr, field);
        return hash_t::get_value((HashTable*) value->value_data.ptr, &fieldStr);
    }
} // namespace tagged_value_wrappers

void setup_type(Type* type)
{
    type->initialize = tagged_value_wrappers::initialize;
    type->release = tagged_value_wrappers::release;
    type->copy = tagged_value_wrappers::copy;
    type->toString = tagged_value_wrappers::to_string;
    type->getField = tagged_value_wrappers::get_field;
    type->name = "Map";
}

} // namespace hash_t
} // namespace circa
