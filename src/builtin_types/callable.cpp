// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "type.h"

namespace circa {
namespace callable_t {

    bool static_type_match(Type* type, Term* term)
    {
        return term->type == FUNCTION_TYPE
            || term->type == TYPE_TYPE;
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->staticTypeMatch = static_type_match;
    }
}
}
