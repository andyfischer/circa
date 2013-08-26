// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(caValue* left, caValue* right);
void string_append(caValue* left, const char* right);

// Convert 'right' to a string (if necessary) and append it. If 'right' is
// itself a string value, it will appear with quote marks.
void string_append_quoted(caValue* left, caValue* right);

// Append an integer value as a string.
void string_append(caValue* left, int value);

void string_append_char(caValue* left, char c);

void string_append_qualified_name(caValue* left, caValue* right);

void string_resize(caValue* s, int length);
bool string_eq(caValue* s, const char* str);
bool string_eq(caValue* s, caValue* rhs);
bool string_equals(caValue* left, caValue* right);

// Return true if s equals the empty string, or is a null value.
bool string_empty(caValue* s);

bool string_starts_with(caValue* s, const char* beginning);
bool string_ends_with(caValue* s, const char* str);
void string_remove_suffix(caValue* s, const char* str);
char string_get(caValue* s, int index);
int string_length(caValue* s);
bool string_less_than(caValue* left, caValue* right);
void string_prepend(caValue* result, caValue* prefix);
void string_prepend(caValue* result, const char* prefix);
void string_slice(caValue* s, int start, int end, caValue* out);
int string_find_char(caValue* s, int start, char c);
int string_find_char_from_end(caValue* s, char c);

void string_split(caValue* s, char sep, caValue* listOut);

const char* as_cstring(caValue* value);

// Initialize a string with the given length, and return the address. This value
// can be safely modified until it's shared (copied).
char* string_initialize(caValue* value, int length);

// Deprecated
std::string as_string(caValue* value);

void set_string(caValue* value, const char* s);
void set_string(caValue* value, const char* s, int length);

char* circa_strdup(const char* s);

} // namespace circa
