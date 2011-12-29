// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct String : public TaggedValue
{
};

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(TaggedValue* left, TaggedValue* right);
void string_append(TaggedValue* left, const char* right);
void string_resize(TaggedValue* s, int length);
bool string_eq(TaggedValue* s, const char* str);
bool string_ends_with(TaggedValue* s, const char* str);

const char* as_cstring(TaggedValue* value);
std::string const& as_string(TaggedValue* value);

} // namespace circa
