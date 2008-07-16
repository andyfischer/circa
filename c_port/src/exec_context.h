#ifndef __TERM_EXECUTION_CONTEXT_INCLUDED__
#define __TERM_EXECUTION_CONTEXT_INCLUDED__

struct CircaObject;
struct Term;

struct ExecContext
{
    Term* target;

    ExecContext(Term* term)
    {
        target = term;
    }

    Term* inputTerm(int index);
    CircaObject* input(int index);
    CircaObject* result();
};

#endif
