// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {
    struct Type;
}

union caValueData {
    int asint;
    float asfloat;
    bool asbool;
    void* ptr;
};

struct caValue
{
    caValueData value_data;
    circa::Type* value_type;

    caValue();
    ~caValue();
    caValue(caValue const&);
    caValue(circa::Type* type);
    caValue& operator=(caValue const& rhs);

    void reset();
    std::string toString();
    inline caValue* operator[](int index) { return getIndex(index); }
    caValue* getIndex(int index);
    caValue* getField(const char* fieldName);
    caValue* getField(std::string const& fieldName);
    int numElements();
    bool equals(caValue* rhs);

    // Convenient accessors
    int asInt();
    float asFloat();
    float toFloat();
    const char* asCString();
    std::string const& asString();
    bool asBool();
    circa::Term* asRef();

    // Convenient constructors
    static caValue fromInt(int i);
    static caValue fromFloat(float f);
    static caValue fromString(const char* s);
    static caValue fromBool(bool b);
};

namespace circa {

struct Value : public caValue
{
};

// Initialize this caValue to a null value. This should only be used if the caValue
// contains invalid data, or has a type that does not have a destructor.
void initialize_null(caValue* value);

// Call the type's create() function to initialize 'value' to a new instance of the type.
void create(Type* type, caValue* value);

void change_type(caValue* v, Type* t);

// Set this value to null. This will call the type's destructor if necessary.
void set_null(caValue* value);

// Reset this value to the type's default value (if defined). The value will retain the
// same type.
void reset(caValue* value);

// Full version of cast(). Attempt to cast 'source' to 'type' and store the result
// in 'dest'. If source has a different type, then we'll ask 'type' how to do this
// conversion. 'result' stores whether the operation succedded.
//
// If 'checkOnly' is true, then we won't actually write anything to 'dest', we'll just
// return whether the operation would have succeeded.
void cast(CastResult* result, caValue* source, Type* type, caValue* dest, bool checkOnly);

// Attempts to cast the value to the given type, returns whether it was successful.
bool cast(caValue* source, Type* type, caValue* dest);

// Returns whether this value can be cast to the given type.
bool cast_possible(caValue* value, Type* type);

// Copy 'source' to 'dest'.
void copy(caValue* source, caValue* dest);

// Swap values between 'left' and 'right'. This is cheaper than copy(), because nothing
// special needs to happen. (In comparison, copy() will cause some types to allocate a
// new piece of memory).
void swap(caValue* left, caValue* right);

// Move the contents of 'source' to 'dest', and set 'source' to null. The previous value
// of 'dest' will be destroyed.
void move(caValue* source, caValue* dest);

// touch() is called immediately before modifying a value. If the type implements a
// persistent data structure, then calling this function will ensure that 'value' owns a
// mutable copy of the data. This is only necessary for some types.
void touch(caValue* value);

// Return a string representation of the value.
std::string to_string(caValue* value);

// For a list type, return a string representation where each element is annotated with its
// type name.
std::string to_string_annotated(caValue* value);

// Equality checking. The check is dispatched on the type of lhs, so we can't guarantee that
// this call is reflexive. (but, the type implementor should preserve this property).
bool equals(caValue* lhs, caValue* rhs);

// Equality checking against unboxed types.
bool equals_string(caValue* value, const char* s);
bool equals_int(caValue* value, int i);
bool equals_name(caValue* value, caName name);

// Get an element by index. Dispatched on type, the default behavior is to return NULL.
caValue* get_index(caValue* value, int index);

// Set an element by index. Dispatched on type. Calling this on a function that doesn't
// support it will trigger an internal error.
void set_index(caValue* value, int index, caValue* element);

caValue* get_field(caValue* value, const char* field);
void set_field(caValue* value, const char* field, caValue* element);

int num_elements(caValue* value);

void set_pointer(caValue* value, Type* type, void* p);
void set_pointer(caValue* value, void* ptr);

void* get_pointer(caValue* value);
void* get_pointer(caValue* value, Type* expectedType);

// Type checking against builtin types.
bool is_bool(caValue* value);
bool is_branch(caValue* value);
bool is_error(caValue* value);
bool is_float(caValue* value);
bool is_function(caValue* value);
bool is_function_pointer(caValue* value);
bool is_int(caValue* value);
bool is_list(caValue* value);
bool is_null(caValue* value);
bool is_number(caValue* value);
bool is_opaque_pointer(caValue* value);
bool is_ref(caValue* value);
bool is_string(caValue* value);
bool is_name(caValue* value);
bool is_type(caValue* value);

// Unboxing using builtin types.
bool        as_bool(caValue* value);
Branch*     as_branch(caValue* value);
const char* as_cstring(caValue* value);
float       as_float(caValue* value);
Function*   as_function(caValue* value);
Term*       as_function_pointer(caValue* value);
int         as_int(caValue* value);
List*       as_list(caValue* value);
void*       as_opaque_pointer(caValue* value);
std::string const& as_string(caValue* value);
Type*       as_type(caValue* value);

// Boxing using builtin types.
void set_bool(caValue* value, bool b);
void set_branch(caValue* value, Branch* branch);
Dict* set_dict(caValue* value);
void set_float(caValue* value, float f);
void set_function_pointer(caValue* value, Term* function);
void set_int(caValue* value, int i);
List* set_list(caValue* value);
List* set_list(caValue* value, int size);
void set_opaque_pointer(caValue* value, void* addr);
void set_string(caValue* value, const char* s);
void set_string(caValue* value, std::string const& s);
void set_type(caValue* value, Type* type);

// Complex unboxing functions.
float to_float(caValue* value);
int to_int(caValue* value);

// A 'transient' value is a tagged value that is not initialized/released by the
// type. When the caller is finished with it, it must be set to null using
// cleanup_transient_value(). The caller must guarantee that the value will stay
// valid in between these two calls.
void set_transient_value(caValue* value, void* v, Type* t);
void cleanup_transient_value(caValue* value);

} // namespace circa
