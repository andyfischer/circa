// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

CodeIterator::CodeIterator(Branch* branch)
  : _topBranch(branch), _topIndex(0), _nestedIterator(NULL)
{
    postAdvance();
}

CodeIterator::~CodeIterator()
{
    delete _nestedIterator;
}

void CodeIterator::reset(Branch* branch)
{
    _topBranch = branch;
    _topIndex = 0;

    delete _nestedIterator;
    _nestedIterator = NULL;

    postAdvance();
}

Term* CodeIterator::current()
{
    assert(!finished());
    if (_nestedIterator != NULL)
        return _nestedIterator->current();
    else
        return _topBranch->get(_topIndex);
}

void CodeIterator::advance()
{
    assert(!finished());
    if (_nestedIterator != NULL)
        _nestedIterator->advance();

    else {
        // Check to start a sub-branch
        Branch* innerBranch = get_inner_branch(current());

        if (innerBranch != NULL) {
            _nestedIterator = new CodeIterator(innerBranch);
        } else {
            _topIndex++;
        }
    }
    postAdvance();
}

void CodeIterator::postAdvance()
{
    if (_nestedIterator != NULL) {
        if (_nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;
            _topIndex++;
            postAdvance();
        }

    } else {
        if (_topIndex >= _topBranch->numTerms())
            _topBranch = NULL;
    }
}

} // namespace circa
