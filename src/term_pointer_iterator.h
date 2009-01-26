// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_POINTER_ITERATOR
#define CIRCA_TERM_POINTER_ITERATOR

#include "pointer_iterator.h"

namespace circa {

class TermExternalPointersIterator : public PointerIterator
{
private:
    enum Step { INPUTS, FUNCTION, TYPE, INSIDE_VALUE };

    Term* _term;
    Step _step;
    int _stepIndex;
    PointerIterator* _nestedIterator;

public:
    TermExternalPointersIterator();
    TermExternalPointersIterator(Term*);
    ~TermExternalPointersIterator();

    void start(Term*);
    virtual Term* current();
    virtual void advance();
    virtual bool finished();

private:
    void advanceIfStateIsInvalid();
};

} // namespace circa

#endif
