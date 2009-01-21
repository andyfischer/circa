// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "term.h"
#include "term_pointer_iterator.h"

namespace circa {

TermPointerIterator::TermPointerIterator()
  : _currentTerm(NULL), _step(INPUTS)
{
}

TermPointerIterator::TermPointerIterator(Term* term)
  : _currentTerm(NULL), _step(INPUTS)
{
    start(term);
}

void TermPointerIterator::start(Term* term)
{
    _currentTerm = term;
    _step = INPUTS;
    _stepIndex = 0;
    advanceToValidPointer();
}

Term*& TermPointerIterator::current()
{
    switch (_step) {
    case INPUTS:
        return _currentTerm->inputs[_stepIndex];
    case FUNCTION:
        return _currentTerm->function;
    case TYPE:
        return _currentTerm->type;
    case INSIDE_VALUE:
        throw std::runtime_error("unimplemented");
    }

    throw std::runtime_error("internal error in TermPointerIterator::current");
}

void TermPointerIterator::advance()
{
    _stepIndex++;
    advanceToValidPointer();
}

void TermPointerIterator::advanceToValidPointer()
{
    switch(_step) {
    case INPUTS:
        if (_stepIndex >= _currentTerm->numInputs()) {
            _step = FUNCTION;
            _stepIndex = 0;
        }
        return;
    case FUNCTION:
        if (_stepIndex > 0) {
            _step = TYPE;
            _stepIndex = 0;
        }
        return;
    case TYPE:
        if (_stepIndex > 0) {
            _step = INSIDE_VALUE;
            _stepIndex = 0;
        }
        // fall through
    case INSIDE_VALUE:
        _currentTerm = NULL;
    }
}

bool TermPointerIterator::finished() const
{
    return _currentTerm == NULL;
}

} // namespace circa
