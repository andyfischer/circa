#ifndef CIRCA__BRANCH__INCLUDED
#define CIRCA__BRANCH__INCLUDED

#include "common_headers.h"

#include "term.h"
#include "term_namespace.h"

struct Branch
{
    TermList terms;
    TermNamespace names;
    Term owningTerm;

    void append(Term term);

    // Returns true if there is a term with the given name
    bool containsName(string name);

    // Get the term with the given name. May throw NameNotFound
    Term getNamed(string name);

    // Bind a name to a term
    void bindName(Term term, string name);
};

void initialize_branch(Branch* kernel);

#endif
