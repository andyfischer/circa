// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Accessors
bool is_handle(caValue* value);
void* handle_get_value(caValue* handle);
void handle_set(caValue* box, void* value, caReleaseFunc release);

void handle_setup_type(Type* type);

} // namespace circa
