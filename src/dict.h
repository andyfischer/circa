// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

struct DictData;

namespace dict_t {

    typedef void (*DictVisitor)(void* context, const char* key, TValue* value);

    DictData* create_dict(int capacity);
    DictData* create_dict();
    void free_dict(DictData* data);
    DictData* grow(DictData* data, int new_capacity);
    void grow(DictData** dataPtr);
    DictData* duplicate(DictData* original);
    int insert(DictData** dataPtr, const char* key);
    void insert_value(DictData** dataPtr, const char* key, TValue* value);
    int find_key(DictData* data, const char* key);
    TValue* get_value(DictData* data, const char* key);
    TValue* get_index(DictData* data, int index);
    void remove(DictData* data, const char* key);
    int count(DictData* data);
    void clear(DictData* data);
    void visit_sorted(DictData* data, DictVisitor visitor, void* context);
    void debug_print(DictData* data);

    void iterator_start(DictData* data, TValue* iterator);
    void iterator_next(DictData* data, TValue* iterator);
    void iterator_get(DictData* data, TValue* iterator,
            const char** key, TValue** value);

    void setup_type(Type*);
} // namespace dict_t

// C++ wrapper
struct Dict : TValue
{
    Dict();

    std::string toString();
    TValue* get(const char* key);
    TValue* operator[](const char* key);
    bool contains(const char* key);
    TValue* insert(const char* key);
    void remove(const char* key);
    void set(const char* key, TValue* value);
    void clear();
    bool empty();

    void iteratorStart(TValue* iterator);
    void iteratorNext(TValue* iterator);
    void iteratorGet(TValue* iterator, const char** key, TValue** value);
    void iteratorDelete(TValue* iterator);
    bool iteratorFinished(TValue* iterator);

    void setString(const char* key, const char* value);
    const char* getString(const char* key, const char* defaultTValue);
    void setInt(const char* key, int value);
    int getInt(const char* key, int defaultTValue);

    static Dict* checkCast(TValue* value);

    // lazyCast will cast value to Dict if it's the wrong type. If it's already a Dict,
    // its contents are unchanged.
    static Dict* lazyCast(TValue* value);

    static Dict* cast(TValue* value);
};

// DictData functions
TValue* dict_get(DictData* data, const char* key);
TValue* dict_insert(DictData** dataPtr, const char* key);
std::string dict_to_string(DictData* data);

// TValue functions
Dict* make_dict(TValue* value);
bool is_dict(TValue* value);
Dict* as_dict(TValue* value);

TValue* dict_get(TValue* dict, const char* field);
TValue* dict_insert(TValue* dict, const char* field);

} // namespace circa
