// Copyright 2008 Paul Hodge

#ifndef CIRCA_POINTER_ITERATOR_INCLUDED
#define CIRCA_POINTER_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class PointerIterator
{
public:
    virtual ~PointerIterator() {}

    // Return the current pointer. It's invalid to call this if finished()
    // would return true.
    virtual Term* current() = 0;

    // Advance to the next pointer. It's invalid to call this if finished()
    // would return true. This call may cause this iterator to become finished.
    virtual void advance() = 0;

    // Returns whether we are finished.
    virtual bool finished() = 0;
};

} // namespace circa

#endif
