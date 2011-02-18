// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "branch.h"
#include "term.h"
#include "term.h"

#include "upward_iterator.h"

namespace circa {

UpwardIterator::UpwardIterator(Term* startingTerm)
  : _stopAt(NULL)
{
    _branch = startingTerm->owningBranch;
    _index = startingTerm->index;

    // advance once, we don't yield the starting term
    advance();
}

void
UpwardIterator::stopAt(Branch* branch)
{
    _stopAt = branch;
}

bool UpwardIterator::finished()
{
    return _branch == NULL;
}

Term* UpwardIterator::current()
{
    ca_assert(_branch != NULL);
    return _branch->get(_index);
}

void UpwardIterator::advance()
{
    do {
        _index--;

        if (_index < 0) {
            Term* parentTerm = _branch->owningTerm;
            if (parentTerm == NULL) {
                _branch = NULL;
                return;
            }

            _branch = parentTerm->owningBranch;
            _index = parentTerm->index;

            if (_branch == _stopAt)
                _branch = NULL;
            if (_branch == NULL)
                return;
        }
        
        // repeat until we find a non NULL term
    } while (unfinished() && current() == NULL);
}

} // namespace circa
