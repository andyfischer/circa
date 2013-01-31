// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void parse_string_repr(const char* str, caValue* valueOut);
void write_string_repr(caValue* value, caValue* stringOut);

}
