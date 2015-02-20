// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Initialize this Value to a null value. This should only be used if the Value
// contains invalid data, or has a type that does not have a destructor. Calling this
// on a Value that has a valid value will cause a memory leak.
void initialize_null(Value* value);

// Initialize 'value' to a new instance of type 'type'. Uses the type's 'initialize' handler.
void make(Type* type, Value* value);

// Initialize 'value' to have the given type, but does not call the type's 'initialize' handler.
// The value_data is left NULL, and the caller is expected to fill in value_data.
void make_no_initialize(Type* type, Value* value);

void make_no_initialize_ptr(Type* type, Value* value, void* ptr);

Type* get_value_type(Value* v);
int value_type_id(Value* v);

// Set this value to null. This will call the type's destructor if necessary.
void set_null(Value* value);

// Reset this value to the type's default value (if defined). The value will retain the
// same type.
void reset(Value* value);

// Attempt to cast the value to the given type. Value may be modified in-place.
//
// If 'checkOnly' is true, then we won't actually write anything to 'dest', we'll just
// return whether the operation would have succeeded.
void cast(CastResult* result, Value* source, Type* type, bool checkOnly);

// Shorthand 'cast'. Returns whether the cast was successful.
bool cast(Value* source, Type* type);

// Shorthand 'cast'. Returns whether a cast will succeed, does not perform the cast.
bool cast_possible(Value* value, Type* type);

// Copy 'source' to 'dest'.
void copy(Value* source, Value* dest);

// Swap values between 'left' and 'right'.
void swap(Value* left, Value* right);

// Move the value in 'source' to 'dest', destroying the value previously in 'dest'.
void move(Value* source, Value* dest);

// Modify 'value' to be a copy that is safe to deeply modify.
void touch(Value* value);

// Return whether a touch() is actually necessary for this value. If this returns false,
// the value is already safe to deeply modify.
bool touch_is_necessary(Value* value);

// Obtain a string representation of a value
void to_string(Value* value, Value* out);

// For a list type, return a string representation where each element is annotated with its
// type name.
std::string to_string_annotated(Value* value);

// Equality checking. The check is dispatched on the type of lhs, so we can't guarantee that
// this call is reflexive. (but, the type implementor should preserve this property).
bool equals(Value* lhs, Value* rhs);

// Equality checking against unboxed types.
bool equals_string(Value* value, const char* s);
bool equals_int(Value* value, int i);

bool strict_equals(Value* left, Value* right);

// Order comparison using default sorting. Returns -1 if 'left' should occur first, 1 if
// 'right' should occur first, and 0 if they have equal order.
// Warning: This is a half-implemented function which only works reliably for
// values of bool/int/float/string.
int compare(Value* left, Value* right);

// Get an element by index. Dispatched on type, the default behavior is to return NULL.
Value* get_index(Value* value, int index);

// Set an element by index. Dispatched on type. Calling this on a function that doesn't
// support it will trigger an internal error.
void set_index(Value* value, int index, Value* element);

Value* get_field(Value* value, Value* field, Value* error);
Value* get_field(Value* value, const char* field, Value* error=NULL);

int num_elements(Value* value);

bool value_hashable(Value* value);
int get_hash_value(Value* value);

void set_pointer(Value* value, void* ptr);

void* get_pointer(Value* value);
void* get_pointer(Value* value, Type* expectedType);

// Type checking against builtin types.
bool is_blob(Value* value);
bool is_bool(Value* value);
bool is_block(Value* value);
bool is_error(Value* value);
bool is_float(Value* value);
bool is_func(Value* value);
bool is_hashtable(Value* value);
bool is_int(Value* value);
bool is_stack(Value* value);
bool is_list(Value* value);
bool is_list_based(Value* value);
bool is_struct(Value* value);

bool is_null(Value* value);
bool is_number(Value* value);
bool is_opaque_pointer(Value* value);
bool is_ref(Value* value);
bool is_string(Value* value);
bool is_symbol(Value* value);
bool is_term_ref(Value* val);
bool is_type(Value* value);

bool is_list_with_length(Value* value, int length);

// A 'leaf' value does not contain or reference any other values. Examples: int,float,bool,string.
bool is_leaf_value(Value* value);

// Unboxing using builtin types.
bool        as_bool(Value* value);
Block*      as_block(Value* value);
const char* as_cstring(Value* value);
float       as_float(Value* value);
int         as_int(Value* value);
Symbol      as_symbol(Value* value);
void*       as_opaque_pointer(Value* value);
Stack*      as_stack(Value* value);
Term*       as_term_ref(Value* val);
Type*       as_type(Value* value);

bool as_bool_opt(Value* value, bool defaultValue);

void set_value(Value* target, Value* value);

// Boxing using builtin types.
void set_bool(Value* value, bool b);
void set_true(Value* value);
void set_false(Value* value);
void set_block(Value* value, Block* block);
void set_error_string(Value* value, const char* s);
void set_float(Value* value, float f);
void set_hashtable(Value* value);
void set_int(Value* value, int i);
Value* set_list(Value* value);
Value* set_list(Value* value, int size);
Value* set_list_1(Value* value, Value* item1);
void set_opaque_pointer(Value* value, void* addr);
void set_string(Value* value, const char* s);
void set_string(Value* value, std::string const& s);
void set_symbol(Value* value, Symbol val);
void set_term_ref(Value* val, Term* term);
void set_type(Value* value, Type* type);

// Complex unboxing functions.
float to_float(Value* value);
int to_int(Value* value);

// If the value is a symbol, return it. If it's a value and the first element is a symbol,
// then return that. Will recursively search if the first element is itself a list.
// Returns s_none if a symbol value was not found.
Symbol first_symbol(Value* value);

} // namespace circa
