// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

#include "common_headers.h"

struct Branch;
struct Term;

namespace circa {

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
