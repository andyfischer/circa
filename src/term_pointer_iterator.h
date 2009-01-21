// Copyright 2008 Paul Hodge

#ifndef CIRCA_TERM_POINTER_ITERATOR
#define CIRCA_TERM_POINTER_ITERATOR

namespace circa {

class TermPointerIterator
{
private:
    enum Step { INPUTS, FUNCTION, TYPE, INSIDE_VALUE };

    Term* _currentTerm;
    Step _step;
    int _stepIndex;

    void advanceToValidPointer();

public:
    TermPointerIterator();
    TermPointerIterator(Term*);

    void start(Term*);
    Term*& current();
    void advance();
    bool finished() const;
};

} // namespace circa

#endif
