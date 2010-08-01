// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "tagged_value.h"

namespace circa {

namespace dict_t {

    struct DictData;

    DictData* create_dict(int capacity);
    DictData* create_dict();
    void free_dict(DictData* data);
    int insert(DictData** dataPtr, const char* key);
    void insert_value(DictData** dataPtr, const char* key, TaggedValue* value);
    int find_key(DictData* data, const char* key);
    TaggedValue* get_value(DictData* data, const char* key);
    TaggedValue* get_index(DictData* data, int index);
    void remove(DictData* data, const char* key);
    DictData* grow(DictData* data, int new_capacity);
    void grow(DictData** dataPtr);
    int count(DictData* data);
    void debug_print(DictData* data);

    void setup_type(Type*);
} // namespace dict_t

} // namespace circa
