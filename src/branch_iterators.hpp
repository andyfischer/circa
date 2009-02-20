// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "introspection.h"

namespace circa {

class BranchIterator : public PointerIterator
{
    Branch* _branch;
    int _index;

public:
    BranchIterator(Branch* branch)
      : _branch(branch), _index(0)
    {
        postAdvance();
    }

    Term* current()
    {
        assert(!finished());
        return _branch->get(_index);
    }

    void advance()
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

    bool finished()
    {
        return _branch == NULL;
    }
};

class RecursiveBranchIterator : public PointerIterator
{
    Branch* _topBranch;
    int _topIndex;
    RecursiveBranchIterator* _subBranch;

public:
    RecursiveBranchIterator(Branch* branch)
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

    ~RecursiveBranchIterator()
    {
        delete _subBranch;
    }

    Term* current()
    {
        assert(!finished());
        if (_subBranch != NULL)
            return _subBranch->current();
        else
            return _topBranch->get(_topIndex);
    }

    void advance()
    {
        assert(!finished());
        if (_subBranch != NULL)
            _subBranch->advance();

        else {
            // Check to start a sub-branch
            Branch* innerBranch = get_inner_branch(current());

            if (innerBranch != NULL) {
                _subBranch = new RecursiveBranchIterator(innerBranch);
            } else {
                _topIndex++;
            }
        }
        postAdvance();
    }

    void postAdvance()
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

    bool finished()
    {
        return _topBranch == NULL;
    }
};

class BranchExternalPointerIterator : public PointerIterator
{
private:
    Branch* _branch;
    int _index;
    TermExternalPointersIterator* _nestedIterator;

public:
    BranchExternalPointerIterator(Branch* branch)
      : _branch(branch),
        _index(0),
        _nestedIterator(NULL)
    {
        if (_branch->numTerms() == 0) {
            // Already finished.
            _branch = NULL;
        } else {
            _nestedIterator = new TermExternalPointersIterator(_branch->get(0));
            advanceIfStateIsInvalid();

            while (!finished() && !shouldExposePointer(current()))
                internalAdvance();
        }
    }

    virtual Term* current()
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

            _nestedIterator = new TermExternalPointersIterator(_branch->get(_index));
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

class BranchControlFlowIterator : public PointerIterator
{
    BranchIterator _branchIterator;
    PointerIterator* _nestedIterator;

public:
    BranchControlFlowIterator(Branch* branch)
      : _branchIterator(branch), _nestedIterator(NULL)
    {
        if (!_branchIterator.finished()) {
            _nestedIterator = start_control_flow_iterator(_branchIterator.current());
            advanceIfStateIsInvalid();
        }
    }

    Term* current()
    {
        if (_nestedIterator == NULL)
            return _branchIterator.current();
        else
            return _nestedIterator->current();
    }

    void advance()
    {
        if (_nestedIterator != NULL)
            _nestedIterator->advance();
        else
            _branchIterator.advance();

        advanceIfStateIsInvalid();
    }

    bool finished()
    {
        return _branchIterator.finished();
    }
private:
    void advanceIfStateIsInvalid()
    {
        if (finished())
            return;
        while (_nestedIterator != NULL && _nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;
            _branchIterator.advance();
            if (_branchIterator.finished())
                return;
            _nestedIterator = start_control_flow_iterator(_branchIterator.current());
        }
    }
};

} // namespace circa
