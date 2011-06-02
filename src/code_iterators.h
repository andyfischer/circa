// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

#include <vector>

namespace circa {

struct BranchIterator
{
    struct Frame {
        Branch* branch;
        int index;

        Frame(Branch* b, int i) : branch(b), index(i) {}
    };

    std::vector<Frame> _stack;
    bool _backwards;
    bool _skipNextBranch;

    BranchIterator(Branch* branch, bool backwards=false);
    void reset(Branch* branch);
    bool finished();
    bool unfinished() { return !finished(); }
    Term* current();
    void advance();
    void advanceSkippingBranch();
    int depth();

    // Next call to advance() will not explore a branch, if there is one.
    void skipNextBranch() { _skipNextBranch = true; }

    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

struct UpwardIterator
{
    Branch* _branch;
    int _index;

    Branch* _stopAt;

    UpwardIterator(Term* startingTerm);

    // stopAt is optional, if set then we will not continue past the given branch.
    void stopAt(Branch* branch);

    bool finished();
    Term* current();
    void advance();

    bool unfinished() { return !finished(); }
    Term* operator*() { return current(); }
    Term* operator->() { return current(); }
    void operator++() { advance(); }
};

    
} // namespace circa
