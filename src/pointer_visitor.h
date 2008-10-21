// Copyright 2008 Andrew Fischer

#ifndef CIRCA_POINTER_VISITOR_INCLUDED
#define CIRCA_POINTER_VISITOR_INCLUDED

#include "common_headers.h"

namespace circa {

struct PointerVisitor
{
    virtual ~PointerVisitor() {}
    virtual void visitPointer(Term* term) = 0;
};

} // namespace circa

#endif
