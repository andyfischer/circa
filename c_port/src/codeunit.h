#ifndef __CODEUNIT_INCLUDED__
#define __CODEUNIT_INCLUDED__

class Term;

struct CodeUnit
{
    Term* _bootstrapEmptyTerm();

    void bindName(Term* term, string name);
    void setInput(Term* term, int index, Term* input);
};

#endif
