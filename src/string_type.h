// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(caValue* left, caValue* right);
void string_append(caValue* left, const std::string& right);
void string_append(caValue* left, const char* right);
void string_append_len(caValue* left, const char* right, int len);

// Convert 'right' to a string (if necessary) and append it. If 'right' is
// itself a string value, it will appear with quote marks.
void string_append_quoted(caValue* left, caValue* right);

// Append an integer value as a string.
void string_append(caValue* left, int value);
void string_append_f(caValue* left, float value);

void string_append_char(caValue* left, char c);
void string_append_ptr(caValue* left, void* ptr);

void string_append_qualified_name(caValue* left, caValue* right);

void string_resize(caValue* s, int length);
bool string_equals(caValue* s, const char* str);
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
void string_substr(caValue* s, int start, int len, caValue* out);
void string_slice(caValue* s, int start, int end, caValue* out);
void string_slice(caValue* str, int start, int end);
int string_find_char(caValue* s, int start, char c);
int string_find_char_from_end(caValue* s, char c);
void string_quote_and_escape(caValue* s);
void string_unquote_and_unescape(caValue* s);
void string_join(caValue* list, caValue* separator, caValue* out);

void string_split(caValue* s, char sep, caValue* listOut);

const char* as_cstring(caValue* value);

// Initialize a string with the given length, and return the address. This value
// can be safely modified until it's shared (copied).
char* string_initialize(caValue* value, int length);

// Deprecated
std::string as_string(caValue* value);

void set_string(caValue* value, const char* s);
void set_string(caValue* value, const char* s, int length);

// Manipulate a string as a blob
char* set_blob(caValue* value, int len);
char* as_blob(caValue* val);
void blob_append_char(caValue* blob, char c);
void blob_append_u8(caValue* blob, u8 val);
void blob_append_u16(caValue* blob, u16 val);
void blob_append_u32(caValue* blob, u32 val);
void blob_append_float(caValue* blob, float f);
void blob_append_space(caValue* blob, size_t size);

char blob_read_char(const char* data, u32* pos);
u8 blob_read_u8(const char* data, u32* pos);
u16 blob_read_u16(const char* data, u32* pos);
u32 blob_read_u32(const char* data, u32* pos);
float blob_read_float(const char* data, u32* pos);
void* blob_read_pointer(const char* data, u32* pos);

void blob_write_u8(char* data, u32* pos, u8 value);
void blob_write_u32(char* data, u32* pos, u32 value);
void blob_write_pointer(char* data, u32* pos, void* value);

void blob_to_hex_string(caValue* blob, caValue* str);

char* circa_strdup(const char* s);

} // namespace circa
