// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

BranchIterator::BranchIterator(Branch& branch, bool backwards)
  : _backwards(backwards)
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
    Frame& frame = _stack.back();
    return frame.branch->get(frame.index);
}

void BranchIterator::advance()
{
    assert(!finished());

    Term* term = current();

    // check to start an inner branch
    if (is_branch(term)
            && is_value_alloced(term)
            && (as_branch(term).length() > 0)) {
        int firstIndex = _backwards ? as_branch(term).length() - 1 : 0;
        _stack.push_back(Frame(&as_branch(term), firstIndex));
        return;
    }

    // otherwise, increment index
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
