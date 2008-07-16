
#include "common_headers.h"

#include "bootstrap.h"
#include "codeunit.h"
#include "function.h"
#include "term.h"
#include "type.h"

Term* CodeUnit::_newTerm(Branch* branch)
{
    if (branch == null)
        branch = &mainBranch;

    Term* term = new Term();
    branch->append(term);
    term->owningBranch = branch;
    return term;
}

Term* CodeUnit::_bootstrapEmptyTerm()
{
    return _newTerm(NULL);
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

void CodeUnit::setInput(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

Term* CodeUnit::createTerm(Term* function, TermList inputs, CircaObject* initialValue, 
    Branch* branch)
{
    // Todo: try to reuse an existing term

    Term* term = _newTerm(branch);
    term->function = function;
    term->inputs = inputs;

    Term* outputType = CA_FUNCTION(function)->outputTypes[0];
    Term* stateType = CA_FUNCTION(function)->stateType;

    // Initialize outputValue
    if (initialValue == null)
        term->outputValue = CA_TYPE(outputType)->alloc(outputType);
    else
        term->outputValue = initialValue;

    // Initialize state
    if (stateType == null)
        term->state = null;
    else
        term->state = CA_TYPE(outputType)->alloc(outputType);

    return term;
}

Term* CodeUnit::createConstant(Term* type, CircaObject* initialValue, Branch* branch)
{
    // Fetch the constant function
    Term* constantFunc = createTerm(KERNEL->getNamed("const-generator"), TermList(type), NULL, NULL);

    // Create the term
    return createTerm(constantFunc, TermList(), initialValue, branch);
}

CircaObject* CodeUnit::executeFunction(Term* function, TermList inputs)
{
    Term tempTerm;

    tempTerm.function = function;
    tempTerm.inputs = inputs;

    tempTerm.execute();

    CircaObject* result = tempTerm.outputValue;
    tempTerm.outputValue = NULL;

    return result;
}

