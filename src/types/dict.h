// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace dict_t {

    struct DictData;

    typedef void (*DictVisitor)(void* context, const char* key, TaggedValue* value);

    DictData* create_dict(int capacity);
    DictData* create_dict();
    void free_dict(DictData* data);
    DictData* grow(DictData* data, int new_capacity);
    void grow(DictData** dataPtr);
    DictData* duplicate(DictData* original);
    int insert(DictData** dataPtr, const char* key);
    void insert_value(DictData** dataPtr, const char* key, TaggedValue* value);
    int find_key(DictData* data, const char* key);
    TaggedValue* get_value(DictData* data, const char* key);
    TaggedValue* get_index(DictData* data, int index);
    void remove(DictData* data, const char* key);
    int count(DictData* data);
    void visit_sorted(DictData* data, DictVisitor visitor, void* context);
    std::string to_string(DictData* data);
    void debug_print(DictData* data);

    void iterator_start(DictData* data, TaggedValue* iterator);
    void iterator_next(DictData* data, TaggedValue* iterator);
    void iterator_get(DictData* data, TaggedValue* iterator,
            const char** key, TaggedValue** value);

    void setup_type(Type*);
} // namespace dict_t

// C++ wrapper
struct Dict : TaggedValue
{
    Dict();

    std::string toString();
    TaggedValue* get(const char* key);
    TaggedValue* operator[](const char* key);
    bool contains(const char* key);
    TaggedValue* insert(const char* key);
    void remove(const char* key);
    void set(const char* key, TaggedValue* value);
    void clear();
    bool empty();

    void iteratorStart(TaggedValue* iterator);
    void iteratorNext(TaggedValue* iterator);
    void iteratorGet(TaggedValue* iterator, const char** key, TaggedValue** value);
    bool iteratorFinished(TaggedValue* iterator);

    static Dict* checkCast(TaggedValue* value);

    // lazyCast will cast value to Dict if it's the wrong type. If it's already a Dict,
    // its contents are unchanged.
    static Dict* lazyCast(TaggedValue* value);
};

Dict* make_dict(TaggedValue* value);
bool is_dict(TaggedValue* value);

} // namespace circa
