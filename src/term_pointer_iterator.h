// Copyright 2008 Andrew Fischer

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

    void findNextValidPointer();

public:
    TermPointerIterator();
    TermPointerIterator(Term*);
    ~TermPointerIterator();

    void start(Term*);
    virtual Term*& current();
    virtual void advance();
    virtual bool finished();
};

} // namespace circa

#endif
