
#include <cstdio>
#include <sstream>
#include <iostream>

#include "Branch.h"
#include "CircaObject.h"
#include "Term.h"

Term* Term_createRaw()
{
    return new Term();
}

Term* Term_getInput(Term* term, int index)
{
    return term->inputs[index];
}
Term* Term_getFunction(Term* term)
{
    return term->function;
}
CircaObject* Term_getOutputValue(Term* term)
{
    return term->outputValue;
}
CircaObject* Term_getState(Term* term)
{
    return term->state;
}
void ForeignFunctionTest()
{
    std::cout << "Test successful" << std::endl;
}
