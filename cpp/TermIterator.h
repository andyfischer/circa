#ifndef TERM_ITERATOR_INCLUDED
#define TERM_ITERATOR_INCLUDED

class Term;

class TermIterator
{
public:
    virtual Term* next() = 0;
    virtual Term* get() const = 0;
    virtual bool more() const = 0;

    Term* operator*() const
    {
        return get();
    }

    void operator++()
    {
        next();
    }
};

#endif
