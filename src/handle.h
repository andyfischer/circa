// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

TValue* get_handle_value(TValue* handle);
void set_handle_value(TValue* handle, Type* type, TValue* value, ReleaseFunc releaseFunc);
void setup_handle_type(Type* type);

} // namespace circa
