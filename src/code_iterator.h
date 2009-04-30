// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CODE_ITERATOR_INCLUDED
#define CIRCA_CODE_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class CodeIterator
{
    Branch* _topBranch;
    int _topIndex;
    CodeIterator* _nestedIterator;

public:
    CodeIterator(Branch* branch);
    ~CodeIterator();
    void reset(Branch* branch);
    Term* current();
    void advance();

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }

    bool finished()
    {
        return _topBranch == NULL;
    }

    int depth()
    {
        if (_nestedIterator == NULL)
            return 0;
        else
            return _nestedIterator->depth() + 1;
    }

private:
    void postAdvance();
};
    
} // namespace circa

#endif
