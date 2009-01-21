// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_POINTER_ITERATOR
#define CIRCA_TERM_POINTER_ITERATOR

#include "pointer_iterator.h"

namespace circa {

class TermPointerIterator : public PointerIterator
{
private:
    enum Step { INPUTS, FUNCTION, TYPE, INSIDE_VALUE };

    Term* _term;
    Step _step;
    int _stepIndex;
    PointerIterator* _nestedIterator;

    void advanceToValidPointer();

public:
    TermPointerIterator();
    TermPointerIterator(Term*);

    void start(Term*);
    virtual Term*& current();
    virtual void advance();
    virtual bool finished();
};

} // namespace circa

#endif
