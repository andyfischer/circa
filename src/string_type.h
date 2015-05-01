// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(Value* left, Value* right);
void string_append(Value* left, const std::string& right);
void string_append(Value* left, const char* right);
void string_append_len(Value* left, const char* right, int len);

// Convert 'right' to a string (if necessary) and append it. If 'right' is
// itself a string value, it will appear with quote marks.
void string_append_quoted(Value* left, Value* right);

// Append an integer value as a string.
void string_append(Value* left, int value);
void string_append_f(Value* left, float value);

void string_append_char(Value* left, char c);
void string_append_ptr(Value* left, void* ptr);

void string_resize(Value* s, int length);
bool string_equals(Value* s, const char* str);
bool string_equals(Value* left, Value* right);

// Return true if s equals the empty string, or is a null value.
bool string_empty(Value* s);

bool string_starts_with(Value* s, const char* beginning);
bool string_ends_with(Value* s, const char* str);
void string_remove_suffix(Value* s, const char* str);
char string_get(Value* s, int index);
int string_length(Value* s);
bool string_less_than(Value* left, Value* right);
void string_prepend(Value* result, Value* prefix);
void string_prepend(Value* result, const char* prefix);
void string_substr(Value* s, int start, int len, Value* out);
void string_slice(Value* s, int start, int end, Value* out);
void string_slice(Value* str, int start, int end);
int string_find_char(Value* s, int start, char c);
int string_find_char_from_end(Value* s, char c);
void string_quote_and_escape(Value* s);
void string_unquote_and_unescape(Value* s);
void string_join(Value* list, Value* separator, Value* out);

void string_split(Value* s, char sep, Value* listOut);

const char* as_cstring(Value* value);

// Initialize a string with the given length, and return the address. This value
// can be safely modified until it's shared (copied).
char* string_initialize(Value* value, int length);

// Deprecated
std::string as_string(Value* value);

void set_string(Value* value, const char* s);
void set_string(Value* value, const char* s, int length);

// Manipulate a string as a blob
#if 0
char* set_blob(Value* value, int len);
char* as_blob(Value* val);
void blob_append_char(Value* blob, char c);
void blob_append_u8(Value* blob, u8 val);
void blob_append_u16(Value* blob, u16 val);
void blob_append_u32(Value* blob, u32 val);
void blob_append_float(Value* blob, float f);
void blob_append_space(Value* blob, size_t size);

char blob_read_char(const char* data, u32* pos);
u8 blob_read_u8(const char* data, u32* pos);
u16 blob_read_u16(const char* data, u32* pos);
u32 blob_read_u32(const char* data, u32* pos);
float blob_read_float(const char* data, u32* pos);
void* blob_read_pointer(const char* data, u32* pos);

void blob_write_u8(char* data, u32* pos, u8 value);
void blob_write_u32(char* data, u32* pos, u32 value);
void blob_write_pointer(char* data, u32* pos, void* value);

void blob_to_hex_string(Value* blob, Value* str);
#endif

char* circa_strdup(const char* s);

} // namespace circa
