// Copyright 2008 Andrew Fischer

#ifndef CIRCA_CODE_ITERATOR_INCLUDED
#define CIRCA_CODE_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class CodeIterator
{
    Branch* _topBranch;
    int _topIndex;
    CodeIterator* _subBranch;

public:
    CodeIterator(Branch* branch)
      : _topBranch(branch), _topIndex(0), _subBranch(NULL)
    {
        postAdvance();
    }

    void reset(Branch* branch)
    {
        _topBranch = branch;
        _topIndex = 0;

        delete _subBranch;
        _subBranch = NULL;

        postAdvance();
    }

    ~CodeIterator()
    {
        delete _subBranch;
    }

    Term* current();
    void advance();
    void postAdvance();

    bool finished()
    {
        return _topBranch == NULL;
    }
};
    
} // namespace circa

#endif
