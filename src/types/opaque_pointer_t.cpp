// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace opaque_pointer_t {

    std::string toString(TaggedValue* val)
    {
        char buf[17];
        sprintf(buf, "%p", as_opaque_pointer(val));
        return buf;
    }

    void setup_type(Type* type)
    {
        reset_type(type);
        type->name = "opaque_pointer";
        type->toString = toString;
    }

} // namespace opaque_pointer_t
} // namespace circa
