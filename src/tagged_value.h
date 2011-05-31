// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {


struct TaggedValue
{
    VariantValue value_data;
    Type* value_type;

    TaggedValue();
    ~TaggedValue();
    TaggedValue(TaggedValue const&);
    TaggedValue(Type* type);
    TaggedValue& operator=(TaggedValue const& rhs);

    void initializeNull();

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
    const char* asCString();
    std::string const& asString();
    bool asBool();
    Term* asRef();

    // Convenient constructors
    static TaggedValue fromInt(int i);
    static TaggedValue fromFloat(float f);
    static TaggedValue fromString(const char* s);
    static TaggedValue fromBool(bool b);
};

// Full version of cast(). Attempt to cast 'source' to 'type' and store the result
// in 'dest'. If source has a different type, then we'll ask 'type' how to do this
// conversion. 'result' stores whether the operation succedded. If 'checkOnly' is
// true, then we won't actually write to 'dest', we'll just record whether the
// operation would have succeeded.
void cast(CastResult* result, TaggedValue* source, Type* type, TaggedValue* dest, bool checkOnly);

// Attempts to cast the value to the given type, returns whether it was successful.
bool cast(TaggedValue* source, Type* type, TaggedValue* dest);

// Returns whether this value can be cast to the given type.
bool cast_possible(TaggedValue* value, Type* type);

// Copy 'source' to 'dest'.
void copy(TaggedValue* source, TaggedValue* dest);

// Swap values between 'left' and 'right'. This is much cheaper than copy(),
// because nothing special needs to happen. (In comparison, copy() will cause
// some types to allocate a new piece of memory).
void swap(TaggedValue* left, TaggedValue* right);

// Convenience function, perform a swap or copy between the two values, depending
// on the 'doCopy' flag.
void swap_or_copy(TaggedValue* left, TaggedValue* right, bool doCopy);

void reset(TaggedValue* value);
std::string to_string(TaggedValue* value);
std::string to_string_annotated(TaggedValue* value);
int num_elements(TaggedValue* value);
void touch(TaggedValue* value);
bool equals(TaggedValue* lhs, TaggedValue* rhs);

void change_type(TaggedValue* v, Type* type);
void change_type_no_initialize(TaggedValue* v, Type* t);

TaggedValue* get_index(TaggedValue* value, int index);
void set_index(TaggedValue* value, int index, TaggedValue* element);
TaggedValue* get_field(TaggedValue* value, const char* field);
void set_field(TaggedValue* value, const char* field, TaggedValue* element);

TaggedValue* set_int(TaggedValue* value, int i);
void set_float(TaggedValue* value, float f);
void set_string(TaggedValue* value, const char* s);
void set_string(TaggedValue* value, std::string const& s);
void set_bool(TaggedValue* value, bool b);
void set_ref(TaggedValue* value, Term* t);
List* set_list(TaggedValue* value);
List* set_list(TaggedValue* value, int size);
void set_type(TaggedValue* value, Type* type);
void set_null(TaggedValue* value);
void set_opaque_pointer(TaggedValue* value, void* addr);

void set_pointer(TaggedValue* value, Type* type, void* p);
void set_pointer(TaggedValue* value, void* ptr);

void* get_pointer(TaggedValue* value);
void* get_pointer(TaggedValue* value, Type* expectedType);

int as_int(TaggedValue* value);
float as_float(TaggedValue* value);
std::string const& as_string(TaggedValue* value);
const char* as_cstring(TaggedValue* value);
bool as_bool(TaggedValue* value);
Term* as_ref(TaggedValue* value);
Branch* as_branch_ref(TaggedValue* value);
void* as_opaque_pointer(TaggedValue* value);
Type& as_type(TaggedValue* value);

bool is_int(TaggedValue* value);
bool is_error(TaggedValue* value);
bool is_float(TaggedValue* value);
bool is_number(TaggedValue* value);
bool is_bool(TaggedValue* value);
bool is_string(TaggedValue* value);
bool is_ref(TaggedValue* value);
bool is_opaque_pointer(TaggedValue* value);
bool is_list(TaggedValue* value);
bool is_type(TaggedValue* value);
bool is_null(TaggedValue* value);
bool is_symbol(TaggedValue* value);

float to_float(TaggedValue* value);
int to_int(TaggedValue* value);

// A 'transient' value is a tagged value that is not initialized/released by the
// type. When the caller is finished with it, it must be set to null using
// cleanup_transient_value(). The caller must guarantee that the value will stay
// valid in between these two calls.
void set_transient_value(TaggedValue* value, void* v, Type* t);
void cleanup_transient_value(TaggedValue* value);

} // namespace circa
