// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct String : public TaggedValue
{
};

void string_setup_type(Type* type);

// Append the two strings, saving the result in 'left'.
void string_append(String* left, String* right);
void string_append(String* left, const char* right);
void string_resize(String* s, int length);

const char* as_cstring(TaggedValue* value);
std::string const& as_string(TaggedValue* value);

} // namespace circa
