// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

TValue* get_handle_value(TValue* handle);
void* get_handle_value_opaque_pointer(TValue* handle);
void set_handle_value(TValue* handle, Type* type, TValue* value, ReleaseFunc releaseFunc);
void set_handle_value_opaque_pointer(TValue* handle, Type* type, void* value,
    ReleaseFunc releaseFunc);
void setup_handle_type(Type* type);

} // namespace circa
