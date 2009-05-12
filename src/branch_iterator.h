// Copyright 2008 Paul Hodge

#ifndef CIRCA_BRANCH_ITERATOR_INCLUDED
#define CIRCA_BRANCH_ITERATOR_INCLUDED

#include "common_headers.h"

namespace circa {

class BranchIterator
{
    struct Frame {
        Branch* branch;
        int index;

        Frame(Branch* b, int i) : branch(b), index(i) {}
    };

    std::vector<Frame> _stack;
    bool _backwards;

public:
    BranchIterator(Branch& branch, bool backwards=false);
    void reset(Branch& branch);
    bool finished();
    Term* current();
    void advance();
    int depth();

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};
    
} // namespace circa

#endif
