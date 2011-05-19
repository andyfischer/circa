// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

namespace circa {
namespace handle_t {

    typedef void (*OnRelease)(Value* data);

    void set(Type* type, Value* container, Value* data);
    Value* get(Value* value);
    void setup_type(Type* type);

} // namespace handle_t
} // namespace circa
