// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "branch.h"
#include "term.h"

#include "code_iterators.h"
#include "inspection.h"

namespace circa {

BranchIterator::BranchIterator(Branch* branch, bool backwards)
  : _backwards(backwards),
    _skipNextBranch(false)
{
    reset(branch);
}

void BranchIterator::reset(Branch* branch)
{
    _stack.clear();
    if (branch->length() != 0) {
        int firstIndex = _backwards ? branch->length() - 1 : 0;
        _stack.push_back(IteratorFrame(branch, firstIndex));
    }
}

bool BranchIterator::finished()
{
    return _stack.empty();
}

Term* BranchIterator::current()
{
    ca_assert(!finished());
    IteratorFrame& frame = _stack.back();
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
    if (term && term->nestedContents && term->contents()->length() > 0) {
        Branch* contents = nested_contents(term);
        int firstIndex = _backwards ? contents->length() - 1 : 0;
        _stack.push_back(IteratorFrame(contents, firstIndex));
        return;
    }

    // Otherwise, just advance. PS, it's not really accurate to say that we are "skipping"
    // any branches, because we just checked if there was one.
    advanceSkippingBranch();
}

void BranchIterator::advanceSkippingBranch()
{
    while (true) {
        Branch* branch = _stack.back().branch;
        int& index = _stack.back().index;

        index += _backwards ? -1 : 1;

        if (index < 0 || index >= branch->length())
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

// UpwardsIterator

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

BranchIteratorFlat::BranchIteratorFlat(Branch* branchIn)
 : branch(branchIn), index(0)
{
    advanceWhileInvalid();
}

bool BranchIteratorFlat::finished()
{
    return index >= branch->length();
}
void BranchIteratorFlat::advance()
{
    index++;
    advanceWhileInvalid();
}
void BranchIteratorFlat::advanceWhileInvalid()
{
possibly_invalid:

    if (finished())
        return;

    if (branch->get(index) == NULL) {
        index++;
        goto possibly_invalid;
    }
}
Term* BranchIteratorFlat::current()
{
    return branch->get(index);
}

BranchInputIterator::BranchInputIterator(Branch* branchIn)
 : branch(branchIn)
{
    index = 0;
    inputIndex = 0;
    advanceWhileInvalid();
}

bool
BranchInputIterator::finished()
{
    return index >= branch->length();
}

void BranchInputIterator::advance()
{
    inputIndex++;
    advanceWhileInvalid();
}

void BranchInputIterator::advanceWhileInvalid()
{
possibly_invalid:

    if (finished())
        return;

    Term* current = branch->get(index);

    if (current == NULL) {
        index++;
        inputIndex = 0;
        goto possibly_invalid;
    }

    if (inputIndex >= current->numInputs()) {
        index++;
        inputIndex = 0;
        goto possibly_invalid;
    }

    if (current->input(inputIndex) == NULL) {
        inputIndex++;
        goto possibly_invalid;
    }
}

Term* BranchInputIterator::currentTerm()
{
    return branch->get(index);
}

Term* BranchInputIterator::currentInput()
{
    return branch->get(index)->input(inputIndex);
}

int BranchInputIterator::currentInputIndex()
{
    return inputIndex;
}

OuterInputIterator::OuterInputIterator(Branch* branch)
 : branchInputIterator(branch)
{
    advanceWhileInvalid();
}

bool
OuterInputIterator::finished()
{
    return branchInputIterator.finished();
}

void OuterInputIterator::advance()
{
    branchInputIterator.inputIndex++;
    advanceWhileInvalid();
}

void OuterInputIterator::advanceWhileInvalid()
{
possibly_invalid:
    branchInputIterator.advanceWhileInvalid();

    if (finished())
        return;

    // Only stop on outer inputs
    if (branchInputIterator.currentInput()->owningBranch == branchInputIterator.branch) {
        branchInputIterator.inputIndex++;
        goto possibly_invalid;
    }
}

Term* OuterInputIterator::currentTerm()
{
    return branchInputIterator.currentTerm();
}

Term* OuterInputIterator::currentInput()
{
    return branchInputIterator.currentInput();
}

int OuterInputIterator::currentInputIndex()
{
    return branchInputIterator.currentInputIndex();
}

BranchIterator2::BranchIterator2()
{
    _current = NULL;
    _topBranch = NULL;
}
BranchIterator2::BranchIterator2(Branch* branch)
{
    _current = branch->get(0);
    _topBranch = branch;
}

void BranchIterator2::startAt(Term* term)
{
    if (term == NULL) {
        _current = NULL;
        _topBranch = NULL;
        return;
    }

    _current = term;
    _topBranch = term->owningBranch;
}

void BranchIterator2::advance()
{
    // Possibly iterate through the contents of this branch.
    if (has_nested_contents(_current) && nested_contents(_current)->length() > 0) {
        _current = nested_contents(_current)->get(0);
        return;
    }

    // Advance to next index.
    Branch* branch = _current->owningBranch;
    int index = _current->index + 1;

    // Possibly loop as we pop out of finished branches.
    possibly_invalid:

    if (index >= branch->length() || branch == NULL) {
        // Finished this branch.

        if (branch == _topBranch || branch == NULL) {
            // Finished the iteration.
            _current = NULL;
            _topBranch = NULL;
            return;
        }

        // Advance to the next term in the parent branch.
        Term* parentTerm = branch->owningTerm;
        if (parentTerm == NULL) {
            // No branch parent. It's weird that we hit this case before we reached
            // the topBranch, but anyway, finish the iteration.
            _current = NULL;
            _topBranch = NULL;
            return;
        }

        branch = parentTerm->owningBranch;
        index = parentTerm->index + 1;
        goto possibly_invalid;
    }

    // Skip over NULL terms.
    if (branch->get(index) == NULL) {
        index++;
        goto possibly_invalid;
    }

    // Index is valid. Save the position as a Term*.
    _current = branch->get(index);
}

NameVisibleIterator::NameVisibleIterator(Term* term)
{
    _target = term;

    // Start at the term immediately after the target.
    _iterator.startAt(following_term(_target));
}


void NameVisibleIterator::advance()
{
    _iterator.advance();

    if (_iterator.finished())
        return;

    // If we reach a term with the same name binding as our target, then stop.
    // Our target term is no longer visible.
    if (_iterator.current()->nameSymbol == _target->nameSymbol) {
        _iterator.stop();
        return;
    }
}

} // namespace circa
