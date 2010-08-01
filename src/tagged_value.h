// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

#include "common_headers.h"

namespace circa {

struct TaggedValue
{
    union Data {
        int asint;
        float asfloat;
        bool asbool;
        void* ptr;
    };

    Data value_data;
    Type* value_type;

    TaggedValue();
    ~TaggedValue();
    TaggedValue(TaggedValue const&);
    TaggedValue(Type* type);
    TaggedValue& operator=(TaggedValue const& rhs);

    void init();
    void reset();
    std::string toString();
    inline TaggedValue* operator[](int index) { return getIndex(index); }
    TaggedValue* getIndex(int index);
    TaggedValue* getField(const char* fieldName);
    TaggedValue* getField(std::string const& fieldName);
    int numElements();
    bool equals(TaggedValue* rhs);

    // Convenient accessors
    int asInt();
    float asFloat();
    float toFloat();
    std::string const& asString();
    bool asBool();
    Ref& asRef();

    // Convenient constructors
    static TaggedValue fromInt(int i);
    static TaggedValue fromFloat(float f);
    static TaggedValue fromString(const char* s);
    static TaggedValue fromBool(bool b);
};

void cast(Type* type, TaggedValue* source, TaggedValue* dest);
void cast(TaggedValue* source, TaggedValue* dest);
void copy(TaggedValue* source, TaggedValue* dest);
void swap(TaggedValue* left, TaggedValue* right);
void reset(TaggedValue* value);
std::string to_string(TaggedValue* value);
bool matches_type(Type* type, Term* term);
TaggedValue* get_index(TaggedValue* value, int index);
void set_index(TaggedValue* value, int index, TaggedValue* element);
TaggedValue* get_field(TaggedValue* value, const char* field);
void set_field(TaggedValue* value, const char* field, TaggedValue* element);
int num_elements(TaggedValue* value);
void touch(TaggedValue* value);

void change_type(TaggedValue* v, Type* type);
bool equals(TaggedValue* lhs, TaggedValue* rhs);

void make_int(TaggedValue* value, int i);
void make_float(TaggedValue* value, float f);
void make_string(TaggedValue* value, const char* s);
void make_bool(TaggedValue* value, bool b);
void make_ref(TaggedValue* value, Term* t);
TaggedValue* make_list(TaggedValue* value);
void make_branch(TaggedValue* value);
void make_null(TaggedValue* value);

void set_int(TaggedValue* value, int i);
void set_float(TaggedValue* value, float f);
void set_bool(TaggedValue* value, bool b);
void set_str(TaggedValue* value, const char* s);
void set_str(TaggedValue* value, std::string const& s);
void set_ref(TaggedValue* value, Term* t);
void set_null(TaggedValue* value);
void set_pointer(TaggedValue* value, Type* type, void* p);

void* get_pointer(TaggedValue* value);
void set_pointer(TaggedValue* value, void* ptr);

Type* get_type_value(TaggedValue* value);
Branch* get_branch_value(TaggedValue* value);
int as_int(TaggedValue* value);
float as_float(TaggedValue* value);
std::string const& as_string(TaggedValue* value);
bool as_bool(TaggedValue* value);
void* get_pointer(TaggedValue* value, Type* expectedType);
Ref& as_ref(TaggedValue* value);
Type& as_type(TaggedValue* value);
float to_float(TaggedValue* value);
int to_int(TaggedValue* value);

bool is_int(TaggedValue* value);
bool is_float(TaggedValue* value);
bool is_bool(TaggedValue* value);
bool is_string(TaggedValue* value);
bool is_ref(TaggedValue* value);
bool is_value_branch(TaggedValue* value);
bool is_type(TaggedValue* value);
bool is_value_of_type(TaggedValue* value, Type* type);
bool is_null(TaggedValue* value);

} // namespace circa
