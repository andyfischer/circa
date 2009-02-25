// Copyright 2008 Andrew Fischer

#ifndef CIRCA_REFERENCE_ITERATOR_INCLUDED
#define CIRCA_REFERENCE_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class ReferenceIterator
{
public:
    virtual ~ReferenceIterator() {}

    // Return the current reference. It's invalid to call this if finished()
    // would return true.
    virtual Ref& current() = 0;

    // Advance to the next reference. It's invalid to call this if finished()
    // would return true. This call may cause this iterator to become finished.
    virtual void advance() = 0;

    // Returns whether we are finished.
    virtual bool finished() = 0;
};

// Consume the rest of iterator, and return the results as a RefList
RefList reference_iterator_to_list(ReferenceIterator& iterator);

// Does the same thing as above, but this syntax will *delete* iterator
// when it is finished. This is convenient for writing one-liners.
RefList reference_iterator_to_list(ReferenceIterator* iterator);

} // namespace circa

#endif
