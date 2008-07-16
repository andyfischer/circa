
#include "common_headers.h"

#include "codeunit.h"
#include "term.h"

Term* CodeUnit::_bootstrapEmptyTerm()
{
    Term* term = new Term();
    mainBranch.append(term);
    return term;
}

void CodeUnit::bindName(Term* term, string name)
{
    mainBranch.bindName(term, name);
}

void CodeUnit::setInput(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}
