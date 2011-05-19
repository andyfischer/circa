// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace dict_t {

    struct DictData;

    typedef void (*DictVisitor)(void* context, const char* key, Value* value);

    DictData* create_dict(int capacity);
    DictData* create_dict();
    void free_dict(DictData* data);
    DictData* grow(DictData* data, int new_capacity);
    void grow(DictData** dataPtr);
    DictData* duplicate(DictData* original);
    int insert(DictData** dataPtr, const char* key);
    void insert_value(DictData** dataPtr, const char* key, Value* value);
    int find_key(DictData* data, const char* key);
    Value* get_value(DictData* data, const char* key);
    Value* get_index(DictData* data, int index);
    void remove(DictData* data, const char* key);
    int count(DictData* data);
    void clear(DictData* data);
    void visit_sorted(DictData* data, DictVisitor visitor, void* context);
    std::string to_string(DictData* data);
    void debug_print(DictData* data);

    void iterator_start(DictData* data, Value* iterator);
    void iterator_next(DictData* data, Value* iterator);
    void iterator_get(DictData* data, Value* iterator,
            const char** key, Value** value);

    void setup_type(Type*);
} // namespace dict_t

// C++ wrapper
struct Dict : Value
{
    Dict();

    std::string toString();
    Value* get(const char* key);
    Value* operator[](const char* key);
    bool contains(const char* key);
    Value* insert(const char* key);
    void remove(const char* key);
    void set(const char* key, Value* value);
    void clear();
    bool empty();

    void iteratorStart(Value* iterator);
    void iteratorNext(Value* iterator);
    void iteratorGet(Value* iterator, const char** key, Value** value);
    void iteratorDelete(Value* iterator);
    bool iteratorFinished(Value* iterator);

    void setString(const char* key, const char* value);
    const char* getString(const char* key, const char* defaultValue);
    void setInt(const char* key, int value);
    int getInt(const char* key, int defaultValue);

    static Dict* checkCast(Value* value);

    // lazyCast will cast value to Dict if it's the wrong type. If it's already a Dict,
    // its contents are unchanged.
    static Dict* lazyCast(Value* value);

    static Dict* cast(Value* value);
};

Dict* make_dict(Value* value);
bool is_dict(Value* value);

} // namespace circa
