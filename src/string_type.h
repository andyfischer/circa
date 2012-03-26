// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "tagged_value.h"

namespace circa {

struct String : public Value
{
};

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(caValue* left, caValue* right);
void string_append(caValue* left, const char* right);
void string_resize(caValue* s, int length);
bool string_eq(caValue* s, const char* str);
bool string_starts_with(caValue* s, const char* beginning);
bool string_ends_with(caValue* s, const char* str);
char string_get(caValue* s, int index);
int string_length(caValue* s);
void string_slice(caValue* s, int start, int end, caValue* out);
int string_find_char(caValue* s, int start, char c);
int string_find_char_from_end(caValue* s, char c);

void string_split(caValue* s, char sep, caValue* listOut);

const char* as_cstring(caValue* value);
std::string const& as_string(caValue* value);

void set_string(caValue* value, const char* s, int length);

} // namespace circa
