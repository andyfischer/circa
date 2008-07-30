
#include "builtins.h"
#include "errors.h"
#include "function.h"

Function::Function()
{
    stateType = NULL;
    recycleInput = -1;
    pureFunction = false;
    variableArgs = false;
    initialize = NULL;
    execute = NULL;
}

bool is_function(Term* term)
{
    return (term->type == BUILTIN_FUNCTION_TYPE) || (term->type == BUILTIN_SUBROUTINE_TYPE);
}

Function* as_function(Term* term)
{
    if (!is_function(term))
        throw errors::TypeError(term, BUILTIN_FUNCTION_TYPE);

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
