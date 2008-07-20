
#include "common_headers.h"

#include "builtins.h"
#include "codeunit.h"
#include "errors.h"
#include "function.h"
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

void initialize_term(Term* term, Term* function)
{
    if (term == NULL)
        throw errors::InternalError("Term is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;

    Term* outputType = as_function(function)->outputType;
    Term* stateType = as_function(function)->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");

    // Allocate
    as_type(outputType)->alloc(term);
    term->type = outputType;
}

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* CodeUnit::createTerm(Term* function, TermList inputs, Branch* branch)
{
    // Todo: try to reuse an existing term

    if (branch == NULL)
        branch = &this->mainBranch;

    Term* term = _newTerm(branch);

    initialize_term(term, function);
    set_inputs(term, inputs);

    return term;
}

Term* CodeUnit::createConstant(Term* type, Branch* branch)
{
    // Fetch the constant function
    Term* constantFunc = createTerm(KERNEL->getNamed("const-generator"), TermList(type), NULL);
    constantFunc->execute();

    // Create the term
    return createTerm(constantFunc, TermList(), branch);
}

void CodeUnit_alloc(Term* caller)
{
    caller->value = new CodeUnit();
}

/*
CircaObject* CaCode_executeFunction(Term* function, TermList inputs)
{
    Term tempTerm;

    initialize_term(&tempTerm, function, NULL);
    set_inputs(&tempTerm, inputs);

    tempTerm.execute();

    CircaObject* result = tempTerm.value;
    tempTerm.value = NULL;

    return result;
}*/

void CaCode_bindName(CodeUnit* codeUnit, Term* term, const char* name)
{
    codeUnit->bindName(term, name);
}

Term* CaCode_createTerm(CodeUnit* codeUnit, Term* function, TermList inputs,
        Branch* branch)
{
    return codeUnit->createTerm(function, inputs, branch);
}

Term* CaCode_createConstant(CodeUnit* codeUnit, Term* type,
        Branch* branch)
{
    return codeUnit->createConstant(type, branch);
}

