#ifndef __CODEUNIT_INCLUDED__
#define __CODEUNIT_INCLUDED__

class Term;

#include "branch.h"

struct CodeUnit
{
    Branch mainBranch;

    Term* _bootstrapEmptyTerm();
    void bindName(Term* term, string name);
    void setInput(Term* term, int index, Term* input);
};

#endif
