// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_REFERENCE_ITERATOR
#define CIRCA_TERM_REFERENCE_ITERATOR

#include "reference_iterator.h"

namespace circa {

class TermReferenceIterator : public ReferenceIterator
{
private:
    enum Step { INPUTS, FUNCTION, TYPE, INSIDE_VALUE };

    Term* _term;
    Step _step;
    int _stepIndex;
    ReferenceIterator* _nestedIterator;

public:
    TermReferenceIterator();
    TermReferenceIterator(Term*);
    ~TermReferenceIterator();

    void start(Term*);
    virtual Ref& current();
    virtual void advance();
    virtual bool finished();

private:
    void advanceIfStateIsInvalid();
};

} // namespace circa

#endif
