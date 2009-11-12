// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "common_headers.h"

#include "term.h"

namespace circa {

Term* alloc_term()
{
    // This function is not very useful now, but in the future we may use
    // a pool for term objects. So, don't use "new Term()", call this function.
    return new Term();
}

} // namespace circa
