
#include "function.h"

#define CA_FUNCTION(t) ((CircaFunction*)t->outputValue)

CircaFunction* CaFunction_alloc(Term*)
{
    return new CircaFunction();
}

void CaFunction_setPureFunction(Term* term, bool value)
{
    CA_FUNCTION(term)->pureFunction = value;
}
void CaFunction_setName(Term* term, const char* value)
{
    CA_FUNCTION(term)->name = value;
}
