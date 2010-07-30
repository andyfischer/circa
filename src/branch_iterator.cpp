// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

BranchIterator::BranchIterator(Branch& branch, bool backwards)
  : _backwards(backwards),
    _skipNextBranch(false)
{
    reset(branch);
}

void BranchIterator::reset(Branch& branch)
{
    _stack.clear();
    if (branch.length() != 0) {
        int firstIndex = _backwards ? branch.length() - 1 : 0;
        _stack.push_back(Frame(&branch, firstIndex));
    }
}

bool BranchIterator::finished()
{
    return _stack.empty();
}

Term* BranchIterator::current()
{
    ca_assert(!finished());
    Frame& frame = _stack.back();
    return frame.branch->get(frame.index);
}

void BranchIterator::advance()
{
    ca_assert(!finished());
    if (_skipNextBranch) {
        _skipNextBranch = false;
        advanceSkippingBranch();
        return;
    }

    Term* term = current();

    // Check to start an inner branch.
    Branch& contents = term->nestedContents;
    if (term != NULL && contents.length() > 0) {
        int firstIndex = _backwards ? contents.length() - 1 : 0;
        _stack.push_back(Frame(&contents, firstIndex));
        return;
    }

    // Otherwise, just advance. PS, it's not really accurate to say that we are "skipping"
    // any branches, because we just checked if there was one.
    advanceSkippingBranch();
}

void BranchIterator::advanceSkippingBranch()
{
    while (true) {
        Branch& branch = *_stack.back().branch;
        int& index = _stack.back().index;

        index += _backwards ? -1 : 1;

        if (index < 0 || index >= branch.length())
            _stack.pop_back();
        else
            break;

        if (_stack.empty())
            break;
    }
}

int BranchIterator::depth()
{
    return (int) _stack.size() - 1;
}

} // namespace circa
