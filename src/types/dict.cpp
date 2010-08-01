// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "dict.h"

namespace circa {
namespace dict_t {

struct Slot {
    char* key;
    TaggedValue value;
};

struct DictData {
    int capacity;
    int count;
    Slot slots[0];
    // slots has size [capacity].
};

// How many slots to create for a brand new dictionary.
const int INITIAL_SIZE = 10;

// When reallocating a dictionary, how many slots should initially be filled.
const float INITIAL_LOAD_FACTOR = 0.3;

// The load at which we'll trigger a reallocation.
const float MAX_LOAD_FACTOR = 0.75;

int hash_string(const char* str)
{
    // Dumb and simple hash function
    int result = 0;
    int byte = 0;
    while (*str != 0) {
        result = result ^ (*str << (8 * byte));
        byte = (byte + 1) % 4;
        str++;
    }
    return result;
}

DictData* create_dict(int capacity)
{
    DictData* result = (DictData*) malloc(sizeof(DictData) + capacity * sizeof(Slot));
    result->capacity = capacity;
    memset(result->slots, 0, capacity * sizeof(Slot));
    for (int s=0; s < capacity; s++)
        result->slots[s].value.init();
    return result;
}

DictData* create_dict()
{
    return create_dict(INITIAL_SIZE);
}

void free_dict(DictData* data)
{
    for (int i=0; i < data->capacity; i++) {
        free(data->slots[i].key);
        make_null(&data->slots[i].value);
    }
    free(data);
}

// Get the 'ideal' slot index, the place we would put this string if
// there is no collision.
int find_ideal_slot_index(DictData* data, const char* str)
{
    unsigned int hash = hash_string(str);
    return int(hash % data->capacity);
}

// Insert the given key into the dictionary, returns the index.
// This may create a new DictData* object, don't use the old
// DictData* pointer after calling this.
int insert(DictData** dataPtr, const char* key)
{
    // Check if this key is already here
    int existing = find_key(*dataPtr, key);
    if (existing != -1)
        return existing;

    // Check if it is time to reallocate
    if ((*dataPtr)->count >= MAX_LOAD_FACTOR * ((*dataPtr)->capacity))
        grow(dataPtr);

    DictData* data = *dataPtr;
    int index = find_ideal_slot_index(data, key);

    // Linearly advance if this slot is being used
    int starting_index = index;
    while (data->slots[index].key != NULL) {
        index = (index + 1) % data->capacity;

        // Die if we made it all the way around
        ca_assert(index != starting_index);
    }

    Slot* slot = &data->slots[index];
    slot->key = strdup(key);
    data->count++;
    return index;
}

void insert_value(DictData** dataPtr, const char* key, TaggedValue* value)
{
    int index = insert(dataPtr, key);
    copy(value, &(*dataPtr)->slots[index].value);
}

// This is like strcmp but it doesn't die on null pointers
bool equal_strings(const char* left, const char* right)
{
    if (left == NULL) return false;
    if (right == NULL) return false;
    return strcmp(left, right) == 0;
}

int find_key(DictData* data, const char* key)
{
    int index = find_ideal_slot_index(data, key);
    int starting_index = index;
    while (!equal_strings(key, data->slots[index].key)) {
        index = (index + 1) % data->capacity;

        // If we hit an empty slot then their key isn't there
        if (data->slots[index].key == NULL)
            return -1;

        // Or if we did a loop then also give up
        if (index == starting_index)
            return -1;
    }

    return index;
}

TaggedValue* get_value(DictData* data, const char* key)
{
    int index = find_key(data, key);
    if (index == -1) return NULL;
    return &data->slots[index].value;
}

TaggedValue* get_index(DictData* data, int index)
{
    ca_assert(index < data->capacity);
    ca_assert(data->slots[index].key != NULL);
    return &data->slots[index].value;
}

void remove(DictData* data, const char* key)
{
    int index = find_key(data, key);
    if (index == -1)
        return;

    // Clear out this slot
    free(data->slots[index].key);
    data->slots[index].key = NULL;
    make_null(&data->slots[index].value);
    data->count--;

    // Check if any following keys would prefer to be moved up to this
    // empty slot.
    while (true) {
        int prevIndex = index;
        index = (index+1) % data->capacity;

        Slot* slot = &data->slots[index];
        if (slot->key == NULL)
            break;

        if (find_ideal_slot_index(data, slot->key) != index) {
            Slot* prevSlot = &data->slots[prevIndex];
            prevSlot->key = slot->key;
            slot->key = NULL;
            swap(&slot->value, &prevSlot->value);
            continue;
        }

        break;
    }
}

DictData* grow(DictData* data, int new_capacity)
{
    DictData* new_data = create_dict(new_capacity);

    // Move all the keys & values over.
    for (int i=0; i < data->capacity; i++) {
        Slot* old_slot = &data->slots[i];

        if (old_slot->key == NULL)
            continue;

        int index = insert(&new_data, old_slot->key);
        swap(&old_slot->value, &new_data->slots[index].value);
    }
    return new_data;
}

void grow(DictData** dataPtr)
{
    int new_capacity = int((*dataPtr)->count / INITIAL_LOAD_FACTOR);
    DictData* oldData = *dataPtr;
    *dataPtr = grow(*dataPtr, new_capacity);
    free_dict(oldData);
}

int count(DictData* data)
{
    return data->count;
}

void debug_print(DictData* data)
{
    printf("dict: %p\n", data);
    printf("count: %d, capacity: %d\n", data->count, data->capacity);
    for (int i=0; i < data->capacity; i++) {
        Slot* slot = &data->slots[i];
        const char* key = "<null>";
        if (slot->key != NULL) key = slot->key;
        printf("[%d] %s = %s\n", i, key, to_string(&data->slots[i].value).c_str());
    }
}

} // namespace dict_t
} // namespace circa
