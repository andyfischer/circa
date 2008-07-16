#ifndef __BRANCH_INCLUDED__
#define __BRANCH_INCLUDED__

#include "common_headers.h"
#include "term_namespace.h"

class CodeUnit;
class Term;

struct Branch
{
    std::vector<Term*> terms;
    TermNamespace names;
    Term* owningTerm;
    CodeUnit* owningCodeUnit;

    void append(Term* term);

    // Returns true if there is a term with the given name
    bool containsName(string name);

    // Get the term with the given name. May throw NameNotFound
    Term* getNamed(string name);

    // Bind a name to a term
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
