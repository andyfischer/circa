// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void parse_string_repr(const char* str, int len, Value* out);
void parse_string_repr(Value* str, Value* out);
void write_string_repr(Value* value, Value* stringOut);

} // namespace circa
