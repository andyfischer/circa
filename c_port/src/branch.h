#ifndef __BRANCH_INCLUDED__
#define __BRANCH_INCLUDED__

#include <map>
#include <vector>

class CodeUnit;
class Term;

struct Branch
{
    std::vector<Term*> terms;
    std::map<std::string, Term*> namespace;
    Term* owningTerm;
    CodeUnit* owningCodeUnit;
};

void CaBranch_append(Branch* branch, Term* term);
void CaBranch_clear(Branch* branch);
void CaBranch_containsName(Branch* branch, const char* name);
void CaBranch_getNamed(Branch* branch, const char* name);
void CaBranch_bindName(Branch* branch, Term* term, const char* name);

#endif
