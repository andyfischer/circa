
#include "common_headers.h"

#include "bootstrap.h"
#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "term.h"
#include "type.h"

CodeUnit::CodeUnit()
{
    _type = BUILTIN_CODEUNIT_TYPE;
}

Term* CodeUnit::_newTerm(Branch* branch)
{
    Term* term = new Term();

    if (branch != NULL)
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

void initialize_term(Term* term, Term* function, CircaObject* initialValue)
{
    if (term == NULL)
        throw errors::InternalError("Term is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;

    Term* outputType = CA_FUNCTION(function)->outputTypes[0];
    Term* stateType = CA_FUNCTION(function)->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");

    // Initialize outputValue
    if (initialValue == NULL)
        term->outputValue = CA_TYPE(outputType)->alloc(outputType);
    else
        term->outputValue = initialValue;

    // Initialize state
    if (stateType == NULL)
        term->state = NULL;
    else
        term->state = CA_TYPE(outputType)->alloc(outputType);
}

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* CodeUnit::createTerm(Term* function, TermList inputs, CircaObject* initialValue, 
    Branch* branch)
{
    // Todo: try to reuse an existing term

    if (branch == NULL)
        branch = &this->mainBranch;

    Term* term = _newTerm(branch);

    initialize_term(term, function, initialValue);
    set_inputs(term, inputs);

    return term;
}

Term* CodeUnit::createConstant(Term* type, CircaObject* initialValue, Branch* branch)
{
    // Fetch the constant function
    Term* constantFunc = createTerm(KERNEL->getNamed("const-generator"), TermList(type), NULL, NULL);

    // Create the term
    return createTerm(constantFunc, TermList(), initialValue, branch);
}

CircaObject* CaCode_alloc(Term*)
{
    return new CodeUnit();
}

CircaObject* CaCode_executeFunction(Term* function, TermList inputs)
{
    Term tempTerm;

    initialize_term(&tempTerm, function, NULL);
    set_inputs(&tempTerm, inputs);

    tempTerm.execute();

    CircaObject* result = tempTerm.outputValue;
    tempTerm.outputValue = NULL;

    return result;
}

void CaCode_bindName(CodeUnit* codeUnit, Term* term, const char* name)
{
    codeUnit->bindName(term, name);
}

Term* CaCode_createTerm(CodeUnit* codeUnit, Term* function, TermList inputs,
        CircaObject* initialValue, Branch* branch)
{
    return codeUnit->createTerm(function, inputs, initialValue, branch);
}

Term* CaCode_createConstant(CodeUnit* codeUnit, Term* type,
        CircaObject* initialValue, Branch* branch)
{
    return codeUnit->createConstant(type, initialValue, branch);
}

