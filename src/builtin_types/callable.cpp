// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "type.h"

namespace circa {
namespace callable_t {

    bool matches_type(Type* type, Term* term)
    {
        return term->type == FUNCTION_TYPE
            || term->type == TYPE_TYPE;
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->matchesType = matches_type;
    }
}
}
