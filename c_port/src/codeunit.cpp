
#include "common_headers.h"

#include "builtins.h"
#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "type.h"

CodeUnit::CodeUnit()
{
}

Term* CodeUnit::_newTerm(Branch* branch)
{
    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);
    term->owningBranch = branch;
    return term;
}

void CodeUnit::bindName(Term* term, string name)
{
    mainBranch.bindName(term, name);
}

bool CodeUnit::containsName(string name)
{
    return mainBranch.containsName(name);
}

Term* CodeUnit::getNamed(string name)
{
    return mainBranch.getNamed(name);
}

Term* CodeUnit::createTerm(Term* function, TermList inputs, Branch* branch)
{
    // Todo: try to reuse an existing term

    if (branch == NULL)
        branch = &this->mainBranch;

    Term* term = _newTerm(branch);

    initialize_term(term, function, inputs);

    return term;
}

void CodeUnit_alloc(Term* caller)
{
    caller->value = new CodeUnit();
}

void CaCode_bindName(CodeUnit* codeUnit, Term* term, const char* name)
{
    codeUnit->bindName(term, name);
}

