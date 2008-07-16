#ifndef __BRANCH_INCLUDED__
#define __BRANCH_INCLUDED__

#include "common_headers.h"

class CodeUnit;
class Term;

struct Branch
{
    std::vector<Term*> terms;
    std::map<string, Term*> names;
    Term* owningTerm;
    CodeUnit* owningCodeUnit;

    void append(Term* term);
    void bindName(Term* term, string name);
};

extern "C" {

void CaBranch_append(Branch* branch, Term* term);
void CaBranch_clear(Branch* branch);
void CaBranch_containsName(Branch* branch, const char* name);
void CaBranch_getNamed(Branch* branch, const char* name);
void CaBranch_bindName(Branch* branch, Term* term, const char* name);

}

#endif
