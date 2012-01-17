// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Type;

union TValueData {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

struct TValue
{
    TValueData value_data;
    Type* value_type;

    TValue();
    ~TValue();
    TValue(TValue const&);
    TValue(Type* type);
    TValue& operator=(TValue const& rhs);

    void reset();
    std::string toString();
    inline TValue* operator[](int index) { return getIndex(index); }
    TValue* getIndex(int index);
    TValue* getField(const char* fieldName);
    TValue* getField(std::string const& fieldName);
    int numElements();
    bool equals(TValue* rhs);

    // Convenient accessors
    int asInt();
    float asFloat();
    float toFloat();
    const char* asCString();
    std::string const& asString();
    bool asBool();
    Term* asRef();

    // Convenient constructors
    static TValue fromInt(int i);
    static TValue fromFloat(float f);
    static TValue fromString(const char* s);
    static TValue fromBool(bool b);
};

// Initialize this TValue to a null value. This should only be used if the TValue
// contains invalid data, or has a type that does not have a destructor.
void initialize_null(TValue* value);

// Call the type's create() function to initialize 'value' to a new instance of the type.
void create(Type* type, TValue* value);

void change_type(TValue* v, Type* t);

// Set this value to null. This will call the type's destructor if necessary.
void set_null(TValue* value);

// Reset this value to the type's default value (if defined). The value will retain the
// same type.
void reset(TValue* value);

// Full version of cast(). Attempt to cast 'source' to 'type' and store the result
// in 'dest'. If source has a different type, then we'll ask 'type' how to do this
// conversion. 'result' stores whether the operation succedded.
//
// If 'checkOnly' is true, then we won't actually write anything to 'dest', we'll just
// return whether the operation would have succeeded.
void cast(CastResult* result, TValue* source, Type* type, TValue* dest, bool checkOnly);

// Attempts to cast the value to the given type, returns whether it was successful.
bool cast(TValue* source, Type* type, TValue* dest);

// Returns whether this value can be cast to the given type.
bool cast_possible(TValue* value, Type* type);

// Copy 'source' to 'dest'.
void copy(TValue* source, TValue* dest);

// Swap values between 'left' and 'right'. This is cheaper than copy(), because nothing
// special needs to happen. (In comparison, copy() will cause some types to allocate a
// new piece of memory).
void swap(TValue* left, TValue* right);

// Move the contents of 'source' to 'dest', and set 'source' to null. The previous value
// of 'dest' will be destroyed.
void move(TValue* source, TValue* dest);

// touch() is called immediately before modifying a value. If the type implements a
// persistent data structure, then calling this function will ensure that 'value' owns a
// mutable copy of the data. This is only necessary for some types.
void touch(TValue* value);

// Return a string representation of the value.
std::string to_string(TValue* value);

// For a list type, return a string representation where each element is annotated with its
// type name.
std::string to_string_annotated(TValue* value);

// Equality checking. The check is dispatched on the type of lhs, so we can't guarantee that
// this call is reflexive. (but, the type implementor should preserve this property).
bool equals(TValue* lhs, TValue* rhs);

// Equality checking against unboxed types.
bool equals_string(TValue* value, const char* s);
bool equals_int(TValue* value, int i);

// Get an element by index. Dispatched on type, the default behavior is to return NULL.
TValue* get_index(TValue* value, int index);

// Set an element by index. Dispatched on type. Calling this on a function that doesn't
// support it will trigger an internal error.
void set_index(TValue* value, int index, TValue* element);

TValue* get_field(TValue* value, const char* field);
void set_field(TValue* value, const char* field, TValue* element);

int num_elements(TValue* value);

void set_pointer(TValue* value, Type* type, void* p);
void set_pointer(TValue* value, void* ptr);

void* get_pointer(TValue* value);
void* get_pointer(TValue* value, Type* expectedType);

// Type checking against builtin types.
bool is_bool(TValue* value);
bool is_branch(TValue* value);
bool is_error(TValue* value);
bool is_float(TValue* value);
bool is_function(TValue* value);
bool is_function_pointer(TValue* value);
bool is_int(TValue* value);
bool is_list(TValue* value);
bool is_null(TValue* value);
bool is_number(TValue* value);
bool is_opaque_pointer(TValue* value);
bool is_ref(TValue* value);
bool is_string(TValue* value);
bool is_symbol(TValue* value);
bool is_type(TValue* value);

// Unboxing using builtin types.
bool        as_bool(TValue* value);
Branch*     as_branch(TValue* value);
const char* as_cstring(TValue* value);
float       as_float(TValue* value);
Function*   as_function(TValue* value);
Term*       as_function_pointer(TValue* value);
int         as_int(TValue* value);
List*       as_list(TValue* value);
void*       as_opaque_pointer(TValue* value);
std::string const& as_string(TValue* value);
Type*       as_type(TValue* value);

// Boxing using builtin types.
void set_bool(TValue* value, bool b);
void set_branch(TValue* value, Branch* branch);
Dict* set_dict(TValue* value);
void set_float(TValue* value, float f);
void set_function_pointer(TValue* value, Term* function);
void set_int(TValue* value, int i);
List* set_list(TValue* value);
List* set_list(TValue* value, int size);
void set_opaque_pointer(TValue* value, void* addr);
void set_string(TValue* value, const char* s);
void set_string(TValue* value, std::string const& s);
void set_type(TValue* value, Type* type);

// Complex unboxing functions.
float to_float(TValue* value);
int to_int(TValue* value);

// A 'transient' value is a tagged value that is not initialized/released by the
// type. When the caller is finished with it, it must be set to null using
// cleanup_transient_value(). The caller must guarantee that the value will stay
// valid in between these two calls.
void set_transient_value(TValue* value, void* v, Type* t);
void cleanup_transient_value(TValue* value);

} // namespace circa
