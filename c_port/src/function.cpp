
#include "builtins.h"
#include "function.h"

Function::Function()
{
    stateType = NULL;
    pureFunction = false;
    variableArgs = false;
    initialize = NULL;
    execute = NULL;
}

Function* as_function(Term* term)
{
    // todo: type check
    return (Function*) term->value;
}

void Function_alloc(Term* caller)
{
    caller->value = new Function();
}
void Function_setPureFunction(Term* term, bool value)
{
    as_function(term)->pureFunction = value;
}
void Function_setName(Term* term, const char* value)
{
    as_function(term)->name = value;
}
void Function_setInputType(Term* term, int index, Term* type)
{
    as_function(term)->inputTypes.setAt(index, type);
}
void Function_setOutputType(Term* term, Term* type)
{
    as_function(term)->outputType = type;
}
void Function_setExecute(Term* term, void(*executeFunc)(Term*))
{
    as_function(term)->execute = executeFunc;
}
