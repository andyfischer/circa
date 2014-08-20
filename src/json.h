// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void json_parse(const char* str, Value* valueOut);
void json_write(Value* value, Value* stringOut);

}
