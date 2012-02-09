// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

#include "hashtable.h"

namespace circa {
namespace hashtable_t {

struct Slot {
    TValue key;
    TValue value;
};

struct Hashtable {
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

int get_hash_value(TValue* value)
{
    Type::HashFunc f = value->value_type->hashFunc;
    if (f == NULL) {
        std::string msg;
        msg += "No hash function for type " + value->value_type->name;
        internal_error(msg);
    }
    return f(value);
}

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

        int index = table_insert(&new_data, &old_slot->key, true);
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

        int index = table_insert(&dupe, &slot->key, false);
        copy(&slot->value, &dupe->slots[index].value);
    }
    return dupe;
}

// Get the 'ideal' slot index, the place we would put this key if there is no
// collision.
int find_ideal_slot_index(Hashtable* data, TValue* key)
{
    ca_assert(data->capacity > 0);
    unsigned int hash = get_hash_value(key);
    return int(hash % data->capacity);
}

// Insert the given key into the dictionary, returns the index.
// This may create a new Hashtable* object, so don't use the old Hashtable* pointer after
// calling this.
int table_insert(Hashtable** dataPtr, TValue* key, bool swapKey)
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

    if (swapKey)
        swap(key, &slot->key);
    else
        copy(key, &slot->key);

    data->count++;

    return index;
}

void insert_value(Hashtable** dataPtr, TValue* key, TValue* value)
{
    int index = table_insert(dataPtr, key, false);
    copy(value, &(*dataPtr)->slots[index].value);
}

int find_key(Hashtable* data, TValue* key)
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

TValue* get_value(Hashtable* data, TValue* key)
{
    int index = find_key(data, key);
    if (index == -1) return NULL;
    return &data->slots[index].value;
}

TValue* get_index(Hashtable* data, int index)
{
    ca_assert(index < data->capacity);
    return &data->slots[index].value;
}

void remove(Hashtable* data, TValue* key)
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
            set_null(&slot->key);
            swap(&slot->value, &prevSlot->value);
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
    strm << "[";
    int count = data == NULL ? 0 : data->capacity;
    bool first = true;
    for (int i=0; i < count; i++) {
        if (is_null(&data->slots[i].key))
            continue;

        if (!first)
            strm << ", ";
        first = false;

        strm << to_string(&data->slots[i].key);
        strm << ": " << to_string(&data->slots[i].value);
    }
    strm << "]";
    return strm.str();
}

void debug_print(Hashtable* data)
{
    printf("dict: %p\n", data);
    printf("count: %d, capacity: %d\n", data->count, data->capacity);
    for (int i=0; i < data->capacity; i++) {
        printf("[%d] %s = %s\n", i, to_string(&data->slots[i].key).c_str(),
                to_string(&data->slots[i].value).c_str());
    }
}

void iterator_start(Hashtable* data, TValue* iterator)
{
    if (data == NULL || data->count == 0)
        return set_null(iterator);

    set_int(iterator, 0);

    // Advance if this iterator location isn't valid
    if (is_null(&data->slots[0].key))
        iterator_next(data, iterator);
}

void iterator_next(Hashtable* data, TValue* iterator)
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

void iterator_get(Hashtable* data, TValue* iterator, TValue** key, TValue** value)
{
    int i = as_int(iterator);

    *key = &data->slots[i].key;
    *value = &data->slots[i].value;
}

namespace tagged_value_wrappers {

    void initialize(Type* type, TValue* value)
    {
        value->value_data.ptr = NULL;
    }
    void release(Type*, TValue* value)
    {
        free_table((Hashtable*) value->value_data.ptr);
    }
    void copy(Type* type, TValue* source, TValue* dest)
    {
        change_type(dest, type);
        dest->value_data.ptr = duplicate((Hashtable*) source->value_data.ptr);
    }
    std::string to_string(TValue* value)
    {
        return to_string((Hashtable*) value->value_data.ptr);
    }
    TValue* get_field(TValue* value, const char* field)
    {
        TValue fieldStr;
        set_string(&fieldStr, field);
        return hashtable_t::get_value((Hashtable*) value->value_data.ptr, &fieldStr);
    }
} // namespace tagged_value_wrappers

// Public API
bool is_hashtable(TValue* value)
{
    return value->value_type->initialize == tagged_value_wrappers::initialize;
}

TValue* get_value(TValue* table, TValue* key)
{
    ca_assert(is_hashtable(table));
    return get_value((Hashtable*) table->value_data.ptr, key);
}

void table_insert(TValue* tableTv, TValue* key, TValue* value,
        bool swapKey, bool swapTValue)
{
    ca_assert(is_hashtable(tableTv));
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    int index = table_insert(&table, key, swapKey);

    TValue* slot = &table->slots[index].value;
    if (swapKey)
        swap(value, slot);
    else
        copy(value, slot);
}

void table_remove(TValue* tableTv, TValue* key)
{
    ca_assert(is_hashtable(tableTv));
    Hashtable*& table = (Hashtable*&) tableTv->value_data.ptr;
    remove(table, key);
}

void setup_type(Type* type)
{
    type->initialize = tagged_value_wrappers::initialize;
    type->release = tagged_value_wrappers::release;
    type->copy = tagged_value_wrappers::copy;
    type->toString = tagged_value_wrappers::to_string;
    type->getField = tagged_value_wrappers::get_field;
    type->name = "Map";
}

} // namespace hashtable_t
} // namespace circa
