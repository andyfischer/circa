// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Initialize this caValue to a null value. This should only be used if the caValue
// contains invalid data, or has a type that does not have a destructor. Calling this
// on a caValue that has a valid value will cause a memory leak.
void initialize_null(caValue* value);

// Initialize 'value' to a new instance of type 'type'. Uses the type's 'initialize' handler.
void make(Type* type, caValue* value);

// Initialize 'value' to have the given type, but does not call the type's 'initialize' handler.
// The value_data is left NULL, and the caller is expected to fill in value_data.
void make_no_initialize(Type* type, caValue* value);

Type* get_value_type(caValue* v);

// Set this value to null. This will call the type's destructor if necessary.
void set_null(caValue* value);

// Reset this value to the type's default value (if defined). The value will retain the
// same type.
void reset(caValue* value);

// Attempt to cast the value to the given type. Value may be modified in-place.
//
// If 'checkOnly' is true, then we won't actually write anything to 'dest', we'll just
// return whether the operation would have succeeded.
void cast(CastResult* result, caValue* source, Type* type, bool checkOnly);

// Shorthand 'cast'. Returns whether the cast was successful.
bool cast(caValue* source, Type* type);

// Shorthand 'cast'. Returns whether a cast will succeed, does not perform the cast.
bool cast_possible(caValue* value, Type* type);

// Copy 'source' to 'dest'.
void copy(caValue* source, caValue* dest);

// Swap values between 'left' and 'right'.
void swap(caValue* left, caValue* right);

// Move the value in 'source' to 'dest', destroying the value previously in 'dest'.
void move(caValue* source, caValue* dest);

// Obtain a deep-write-safe reference to 'value'.
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

bool strict_equals(caValue* left, caValue* right);

// Get an element by index. Dispatched on type, the default behavior is to return NULL.
caValue* get_index(caValue* value, int index);

// Set an element by index. Dispatched on type. Calling this on a function that doesn't
// support it will trigger an internal error.
void set_index(caValue* value, int index, caValue* element);

caValue* get_field(caValue* value, caValue* field, caValue* error);
caValue* get_field(caValue* value, const char* field, caValue* error=NULL);

int num_elements(caValue* value);

int get_hash_value(caValue* value);

void set_pointer(caValue* value, void* ptr);

void* get_pointer(caValue* value);
void* get_pointer(caValue* value, Type* expectedType);

// Type checking against builtin types.
bool is_bool(caValue* value);
bool is_block(caValue* value);
bool is_error(caValue* value);
bool is_float(caValue* value);
bool is_func(caValue* value);
bool is_int(caValue* value);
bool is_stack(caValue* value);
bool is_list(caValue* value);
bool is_null(caValue* value);
bool is_number(caValue* value);
bool is_opaque_pointer(caValue* value);
bool is_ref(caValue* value);
bool is_string(caValue* value);
bool is_symbol(caValue* value);
bool is_term_ref(caValue* val);
bool is_type(caValue* value);

// Unboxing using builtin types.
bool        as_bool(caValue* value);
Block*      as_block(caValue* value);
const char* as_cstring(caValue* value);
float       as_float(caValue* value);
int         as_int(caValue* value);
Symbol      as_symbol(caValue* value);
void*       as_opaque_pointer(caValue* value);
Stack*      as_stack(caValue* value);
Term*       as_term_ref(caValue* val);
Type*       as_type(caValue* value);

void set_value(caValue* target, caValue* value);

// Boxing using builtin types.
void set_bool(caValue* value, bool b);
void set_block(caValue* value, Block* block);
Dict* set_dict(caValue* value);
void set_error_string(caValue* value, const char* s);
void set_float(caValue* value, float f);
void set_int(caValue* value, int i);
caValue* set_list(caValue* value);
caValue* set_list(caValue* value, int size);
void set_opaque_pointer(caValue* value, void* addr);
void set_string(caValue* value, const char* s);
void set_string(caValue* value, std::string const& s);
void set_symbol(caValue* value, Symbol val);
void set_term_ref(caValue* val, Term* term);
void set_type(caValue* value, Type* type);

// Complex unboxing functions.
float to_float(caValue* value);
int to_int(caValue* value);

// If the value is a symbol, return it. If it's a value and the first element is a symbol,
// then return that. Will recursively search if the first element is itself a list.
// Returns sym_None if a symbol value was not found.
Symbol first_symbol(caValue* value);

} // namespace circa
