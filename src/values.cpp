// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

void assign_value(Term* source, Term* dest)
{
#if 0
    // New style
    if (as_type(dest->type).assign == NULL) {
        assign_value((TaggedValue*) source, (TaggedValue*) dest);
        return;
    }

    assert(false);
#endif

    // Do a type specialization if dest has type 'any'.
    // (deprecated)
    if (dest->type == ANY_TYPE) {
        //assert(false);
        specialize_type(dest, source->type);
    }

    assign_value((TaggedValue*) source, (TaggedValue*) dest);
    return;
}

} // namespace circa
