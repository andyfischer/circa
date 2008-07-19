
#include "builtins.h"
#include "function.h"

CircaFunction::CircaFunction()
{
    stateType = NULL;
    pureFunction = false;
    variableArgs = false;
    initialize = NULL;
    execute = NULL;
}

CircaFunction* as_function(Term* term)
{
    // todo: type check
    return (CircaFunction*) term->value;
}

void Function_alloc(Term* caller)
{
    caller->value = new CircaFunction();
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
void Function_setOutputType(Term* term, int index, Term* type)
{
    as_function(term)->outputTypes.setAt(index, type);
}
