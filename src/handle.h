// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Accessors
bool is_handle(caValue* value);
caValue* handle_get_value(caValue* handle);

void handle_set_release_func(caValue* handle, ReleaseFunc releaseFunc);
void setup_handle_type(Type* type);
void handle_type_set_release_func(Type* type, ReleaseFunc releaseFunc);

} // namespace circa
