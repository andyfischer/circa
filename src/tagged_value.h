// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

namespace circa {

struct Value
{
    VariantValue value_data;
    Type* value_type;

    #if CIRCA_ENABLE_TAGGED_VALUE_METADATA
    // This metadata is only enabled in debug/test builds.
    //
    // The 'note' is a string describing the purpose of this value. For a temporary
    // value it should describe the location it shows up, for a non-temporary, it
    // should describe the owner and the context.
    std::string metadata_note;
    #endif

    Value();
    ~Value();
    Value(Value const&);
    Value(Type* type);
    Value& operator=(Value const& rhs);

    void init();
    void reset();
    std::string toString();
    inline Value* operator[](int index) { return getIndex(index); }
    Value* getIndex(int index);
    Value* getField(const char* fieldName);
    Value* getField(std::string const& fieldName);
    int numElements();
    bool equals(Value* rhs);

    // Convenient accessors
    int asInt();
    float asFloat();
    float toFloat();
    const char* asCString();
    std::string const& asString();
    bool asBool();
    Term* asRef();

    // Convenient constructors
    static Value fromInt(int i);
    static Value fromFloat(float f);
    static Value fromString(const char* s);
    static Value fromBool(bool b);
};

// Full version of cast(). Attempt to cast 'source' to 'type' and store the result
// in 'dest'. If source has a different type, then we'll ask 'type' how to do this
// conversion. 'result' stores whether the operation succedded. If 'checkOnly' is
// true, then we won't actually write to 'dest', we'll just record whether the
// operation would have succeeded.
void cast(CastResult* result, Value* source, Type* type, Value* dest, bool checkOnly);

// Attempts to cast the value to the given type, returns whether it was successful.
bool cast(Value* source, Type* type, Value* dest);

// Returns whether this value can be cast to the given type.
bool cast_possible(Value* value, Type* type);

// Copy 'source' to 'dest'.
void copy(Value* source, Value* dest);

// Swap values between 'left' and 'right'. This is much cheaper than copy(),
// because nothing special needs to happen. (In comparison, copy() will cause
// some types to allocate a new piece of memory).
void swap(Value* left, Value* right);

// Convenience function, perform a swap or copy between the two values, depending
// on the 'doCopy' flag.
void swap_or_copy(Value* left, Value* right, bool doCopy);

void reset(Value* value);
std::string to_string(Value* value);
std::string to_string_annotated(Value* value);
int num_elements(Value* value);
void touch(Value* value);
bool equals(Value* lhs, Value* rhs);

void change_type(Value* v, Type* type);
void change_type_no_initialize(Value* v, Type* t);

Value* get_index(Value* value, int index);
void set_index(Value* value, int index, Value* element);
Value* get_field(Value* value, const char* field);
void set_field(Value* value, const char* field, Value* element);

Value* set_int(Value* value, int i);
void set_float(Value* value, float f);
void set_string(Value* value, const char* s);
void set_string(Value* value, std::string const& s);
void set_bool(Value* value, bool b);
void set_ref(Value* value, Term* t);
List* set_list(Value* value);
List* set_list(Value* value, int size);
void set_type(Value* value, Type* type);
void set_null(Value* value);
void set_opaque_pointer(Value* value, void* addr);

void set_pointer(Value* value, Type* type, void* p);
void set_pointer(Value* value, void* ptr);

void* get_pointer(Value* value);
void* get_pointer(Value* value, Type* expectedType);

int as_int(Value* value);
float as_float(Value* value);
std::string const& as_string(Value* value);
const char* as_cstring(Value* value);
bool as_bool(Value* value);
Term* as_ref(Value* value);
Branch* as_branch_ref(Value* value);
void* as_opaque_pointer(Value* value);
Type& as_type(Value* value);

bool is_int(Value* value);
bool is_error(Value* value);
bool is_float(Value* value);
bool is_number(Value* value);
bool is_bool(Value* value);
bool is_string(Value* value);
bool is_ref(Value* value);
bool is_opaque_pointer(Value* value);
bool is_list(Value* value);
bool is_type(Value* value);
bool is_value_of_type(Value* value, Type* type);
bool is_null(Value* value);
bool is_symbol(Value* value);

float to_float(Value* value);
int to_int(Value* value);

#if CIRCA_ENABLE_TAGGED_VALUE_METADATA

void debug_set_tagged_value_note(Value* value, std::string const& note);

#else

#define debug_set_tagged_value_note(...)

#endif

} // namespace circa
