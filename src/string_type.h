// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

struct String : public TValue
{
};

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(TValue* left, TValue* right);
void string_append(TValue* left, const char* right);
void string_resize(TValue* s, int length);
bool string_eq(TValue* s, const char* str);
bool string_starts_with(TValue* s, const char* beginning);
bool string_ends_with(TValue* s, const char* str);
char string_get(TValue* s, int index);
int string_length(TValue* s);
void string_slice(TValue* s, int start, int end, TValue* out);
int string_find_char(TValue* s, int start, char c);

void string_split(TValue* s, char sep, TValue* listOut);

const char* as_cstring(TValue* value);
std::string const& as_string(TValue* value);

void set_string(TValue* value, const char* s, int length);

} // namespace circa
