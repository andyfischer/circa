// Copyright 2008 Paul Hodge

#include "branch.h"
#include "introspection.h"
#include "term_reference_iterator.h"

namespace circa {

class BranchIterator : public ReferenceIterator
{
    Branch* _branch;
    int _index;

public:
    BranchIterator(Branch* branch)
      : _branch(branch), _index(0)
    {
        postAdvance();
    }

    virtual Ref& current()
    {
        assert(!finished());
        return _branch->get(_index);
    }

    virtual void advance()
    {
        assert(!finished());
        _index++;
        postAdvance();
    }

    void postAdvance()
    {
        if (_index >= _branch->numTerms())
            _branch = NULL;
    }

    virtual bool finished()
    {
        return _branch == NULL;
    }
};

class BranchExternalReferenceIterator : public ReferenceIterator
{
private:
    Branch* _branch;
    int _index;
    TermReferenceIterator* _nestedIterator;

public:
    BranchExternalReferenceIterator(Branch* branch)
      : _branch(branch),
        _index(0),
        _nestedIterator(NULL)
    {
        if (_branch->numTerms() == 0) {
            // Already finished.
            _branch = NULL;
        } else {
            _nestedIterator = new TermReferenceIterator(_branch->get(0));
            advanceIfStateIsInvalid();

            while (!finished() && !shouldExposePointer(current()))
                internalAdvance();
        }
    }

    virtual Ref& current()
    {
        assert(!finished());
        return _nestedIterator->current();
    }

    virtual bool finished()
    {
        return _branch == NULL;
    }

    virtual void advance()
    {
        assert(!finished());

        // Move forward at least one
        internalAdvance();

        // Now, we're allowed to skip pointers. Specifically, we skip
        // pointers that are internal to our branch, because they are
        // not anyone else's business. We only expose pointers that
        // point outside of this branch.

        while(!finished() && !shouldExposePointer(current()))
            internalAdvance();
    }

private:
    bool shouldExposePointer(Term* ptr) {
        return (ptr != NULL) && current()->owningBranch != _branch;
    }

    void advanceIfStateIsInvalid()
    {
        assert(!finished());

        while (_nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;

            _index++;

            if (_index >= _branch->numTerms()) {
                // finished
                _branch = NULL;
                return;
            }

            _nestedIterator = new TermReferenceIterator(_branch->get(_index));
        }
    }

    // in internalAdvance, we advance by one actual pointer. However, this
    // pointer might not be exposed to the outside world: it might be
    // skipped in the public version of advance().
    
    void internalAdvance()
    {
        assert(!finished());

        _nestedIterator->advance();

        advanceIfStateIsInvalid();
    }
};

} // namespace circa
