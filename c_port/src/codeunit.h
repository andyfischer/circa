#ifndef __CODEUNIT_INCLUDED__
#define __CODEUNIT_INCLUDED__

#include "branch.h"
#include "term.h"

struct CodeUnit
{
    Branch mainBranch;

    // Create a new term on the given branch.
    // 'branch' may be null, in which case we use mainBranch
    Term* _newTerm(Branch* branch);
    Term* _bootstrapEmptyTerm();

    // Returns true if there is a term with the given name
    bool containsName(string name);

    // Get the term with the given name. May throw NameNotFound
    Term* getNamed(string name);

    // Bind a name to a term
    void bindName(Term* term, string name);

    void setInput(Term* term, int index, Term* input);

    // Create a term
    Term* createTerm(Term* function, TermList inputs,
            CircaObject* initialValue, Branch* branch);

    // Create a constant term. Calls createTerm with the appropriate constant
    // function.
    Term* createConstant(Term* type, CircaObject* initialValue, Branch* branch);

    // Execute the given function immediately, and return the result
    CircaObject* executeFunction(Term* function, TermList inputs);
};

#endif
