// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

namespace circa {
namespace simple_handle_t {
    typedef void (*OnRelease)(int handle);

    void set(Type* type, TaggedValue* value, int handle);
    int get(TaggedValue* value);
    void setup_type(Type* type);

} // namespace simple_handle_t
} // namespace circa
