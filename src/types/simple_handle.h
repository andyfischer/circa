// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace simple_handle_t {
    typedef void (*OnRelease)(int handle);

    void set(Type* type, TaggedValue* value, int handle);
    void setup_type(Type* type);
} // namespace simple_handle_t
} // namespace circa
