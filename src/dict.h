// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

struct DictData;

namespace dict_t {

    typedef void (*DictVisitor)(void* context, const char* key, caValue* value);

    DictData* create_dict(int capacity);
    DictData* create_dict();
    void free_dict(DictData* data);
    DictData* grow(DictData* data, int new_capacity);
    void grow(DictData** dataPtr);
    DictData* duplicate(DictData* original);
    int insert(DictData** dataPtr, const char* key);
    void insert_value(DictData** dataPtr, const char* key, caValue* value);
    int find_key(DictData* data, const char* key);
    caValue* get_value(DictData* data, const char* key);
    caValue* get_index(DictData* data, int index);
    void remove(DictData* data, const char* key);
    int count(DictData* data);
    void clear(DictData* data);
    void visit_sorted(DictData* data, DictVisitor visitor, void* context);
    void debug_print(DictData* data);

    void iterator_start(DictData* data, caValue* iterator);
    void iterator_next(DictData* data, caValue* iterator);
    void iterator_get(DictData* data, caValue* iterator,
            const char** key, caValue** value);

    void setup_type(Type*);
} // namespace dict_t

// C++ wrapper
struct Dict : Value
{
    Dict();

    std::string toString();
    caValue* get(const char* key);
    caValue* operator[](const char* key);
    bool contains(const char* key);
    caValue* insert(const char* key);
    void remove(const char* key);
    void set(const char* key, caValue* value);
    void clear();
    bool empty();

    void iteratorStart(caValue* iterator);
    void iteratorNext(caValue* iterator);
    void iteratorGet(caValue* iterator, const char** key, caValue** value);
    void iteratorDelete(caValue* iterator);
    bool iteratorFinished(caValue* iterator);

    const char* getString(const char* key, const char* defaultValue);
    void setString(const char* key, const char* value);
    int getInt(const char* key, int defaultValue);
    void setInt(const char* key, int value);
    bool getBool(const char* key, bool defaultValue);
    void setBool(const char* key, bool value);

    static Dict* checkCast(caValue* value);

    // lazyCast will cast value to Dict if it's the wrong type. If it's already a Dict,
    // its contents are unchanged.
    static Dict* lazyCast(caValue* value);

    static Dict* cast(caValue* value);
};

// DictData functions
caValue* dict_get(DictData* data, const char* key);
caValue* dict_insert(DictData** dataPtr, const char* key);
std::string dict_to_string(DictData* data);

// caValue functions
Dict* make_dict(caValue* value);
bool is_dict(caValue* value);
Dict* as_dict(caValue* value);

caValue* dict_get(caValue* dict, const char* field);
caValue* dict_insert(caValue* dict, const char* field);

} // namespace circa
