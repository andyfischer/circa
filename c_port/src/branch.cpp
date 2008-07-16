
#include "common_headers.h"

#include "branch.h"

void Branch::append(Term* term)
{
    terms.push_back(term);
}

void Branch::bindName(Term* term, string name)
{
    names[name] = term;
}

void CaBranch_bindName(Branch* branch, Term* term, const char* name)
{
    branch->bindName(term, name);
}
