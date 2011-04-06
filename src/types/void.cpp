// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace void_t {
    std::string to_string(TaggedValue*)
    {
        return "<void>";
    }
    void setup_type(Type* type)
    {
        type->name = "void";
        type->toString = to_string;
    }
} // namespace void_t
} // namespace circa
