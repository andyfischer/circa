// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "type.h"

namespace circa {
namespace indexable_t {

    bool matches_type(Type* type, Term* term)
    {
        return declared_type(term)->getIndex != NULL
            && declared_type(term)->numElements != NULL;
    }
    void setup_type(Type* type)
    {
        reset_type(type);
        type->matchesType = matches_type;
    }
}
}
