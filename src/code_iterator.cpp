// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

void CodeIterator::reset(Branch* branch)
{
    _topBranch = branch;
    _topIndex = 0;

    delete _subBranch;
    _subBranch = NULL;

    postAdvance();
}
    
Term* CodeIterator::current()
{
    assert(!finished());
    if (_subBranch != NULL)
        return _subBranch->current();
    else
        return _topBranch->get(_topIndex);
}

void CodeIterator::advance()
{
    assert(!finished());
    if (_subBranch != NULL)
        _subBranch->advance();

    else {
        // Check to start a sub-branch
        Branch* innerBranch = get_inner_branch(current());

        if (innerBranch != NULL) {
            _subBranch = new CodeIterator(innerBranch);
        } else {
            _topIndex++;
        }
    }
    postAdvance();
}

void CodeIterator::postAdvance()
{
    if (_subBranch != NULL) {
        if (_subBranch->finished()) {
            delete _subBranch;
            _subBranch = NULL;
            _topIndex++;
            postAdvance();
        }

    } else {
        if (_topIndex >= _topBranch->numTerms())
            _topBranch = NULL;
    }
}

} // namespace circa
