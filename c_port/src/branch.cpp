
#include "common_headers.h"

#include "branch.h"

void Branch::append(Term* term)
{
    terms.push_back(term);
}

bool Branch::containsName(string name)
{
    return names.contains(name);
}

Term* Branch::getNamed(string name)
{
    return names[name];
}

void Branch::bindName(Term* term, string name)
{
    names[name] = term;
}
