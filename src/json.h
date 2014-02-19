// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

void json_parse(const char* str, caValue* valueOut);
void json_write(caValue* value, caValue* stringOut);

}
