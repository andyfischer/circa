// Copyright 2008 Andrew Fischer

#ifndef CIRCA_POINTER_ITERATOR_INCLUDED
#define CIRCA_POINTER_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class PointerIterator
{
public:
    virtual ~PointerIterator() {}
    virtual Term*& current() = 0;
    virtual void advance() = 0;
    virtual bool finished() = 0;
};

} // namespace circa

#endif
