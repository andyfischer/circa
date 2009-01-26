// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "term.h"
#include "term_pointer_iterator.h"
#include "type.h"

namespace circa {

TermExternalPointersIterator::TermExternalPointersIterator()
  : _term(NULL), _step(INPUTS), _stepIndex(0), _nestedIterator(NULL)
{
}

TermExternalPointersIterator::TermExternalPointersIterator(Term* term)
  : _term(NULL), _step(INPUTS), _stepIndex(0), _nestedIterator(NULL)
{
    start(term);
}

TermExternalPointersIterator::~TermExternalPointersIterator()
{
    delete _nestedIterator;
    _nestedIterator = NULL;
}

void TermExternalPointersIterator::start(Term* term)
{
    _term = term;
    _step = INPUTS;
    _stepIndex = 0;
    _nestedIterator = NULL;
    advanceIfStateIsInvalid();
}

Term* TermExternalPointersIterator::current()
{
    assert(!finished());

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

    throw std::runtime_error("internal error in TermExternalPointersIterator::current");
}

void TermExternalPointersIterator::advance()
{
    if (_step == INSIDE_VALUE)
        _nestedIterator->advance();
    else
        _stepIndex++;

    advanceIfStateIsInvalid();
}

void TermExternalPointersIterator::advanceIfStateIsInvalid()
{
    switch(_step) {
    case INPUTS:
        if (_stepIndex < _term->numInputs())
            return;

        _step = FUNCTION;
        _stepIndex = 0;
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

        if (_nestedIterator == NULL) {
            _term = NULL;
            return;
        }

        // fall through

    case INSIDE_VALUE:
        if (_nestedIterator->finished()) {
            delete _nestedIterator;
            _step = INPUTS;
            _nestedIterator = NULL;
            _term = NULL;
            return;
        }
    }
}

bool TermExternalPointersIterator::finished()
{
    return _term == NULL;
}

} // namespace circa
