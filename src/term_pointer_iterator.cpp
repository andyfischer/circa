// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "term.h"
#include "term_pointer_iterator.h"
#include "type.h"

namespace circa {

TermPointerIterator::TermPointerIterator()
  : _term(NULL), _step(INPUTS), _stepIndex(0), _nestedIterator(NULL)
{
}

TermPointerIterator::TermPointerIterator(Term* term)
  : _term(NULL), _step(INPUTS), _stepIndex(0), _nestedIterator(NULL)
{
    start(term);
}

void TermPointerIterator::start(Term* term)
{
    _term = term;
    _step = INPUTS;
    _stepIndex = 0;
    _nestedIterator = NULL;
    advanceToValidPointer();
}

Term*& TermPointerIterator::current()
{
    if (finished())
        throw std::runtime_error("iterator is finished");

    switch (_step) {
    case INPUTS:
        return _term->inputs[_stepIndex];
    case FUNCTION:
        return _term->function;
    case TYPE:
        return _term->type;
    case INSIDE_VALUE:
        return _nestedIterator->current();
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
        if (_stepIndex >= _term->numInputs()) {
            _step = FUNCTION;
            _stepIndex = 0;
        }
        return;
    case FUNCTION:
        if (_stepIndex == 0)
            return;

        _step = TYPE;
        _stepIndex = 0;
        return;
    case TYPE:

        if (_stepIndex == 0)
            return;

        _step = INSIDE_VALUE;
        _stepIndex = 0;
        _nestedIterator = start_pointer_iterator(_term);

        // fall through

    case INSIDE_VALUE:
        if (_nestedIterator == NULL) {
            _term = NULL;
            _nestedIterator = NULL;
            return;
        }
        
        if (_nestedIterator->finished()) {
            delete _nestedIterator;
            _nestedIterator = NULL;
            _term = NULL;
            return;
        }
    }
}

bool TermPointerIterator::finished()
{
    return _term == NULL;
}

} // namespace circa
