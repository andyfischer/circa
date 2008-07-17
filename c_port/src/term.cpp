
#include <cstdio>
#include <sstream>
#include <iostream>

#include "branch.h"
#include "function.h"
#include "object.h"
#include "term.h"
#include "exec_context.h"


void TermList::setAt(int index, Term* term)
{
    // Make sure there are enough blank elements in the list
    while (items.size() <= index) {
        items.push_back(null);
    }

    items[index] = term;
}
Term* TermList::operator[](int index)
{
    if (index >= items.size())
        return NULL;
    return items[index];
}

Term* Term::getType() const
{
    return this->function->asFunction()->outputTypes[0];
}

void Term::execute()
{
    ExecContext context(this);

    CA_FUNCTION(function)->execute(&context);
}

CircaInt* Term::asInt()
{
    return this->outputValue->asInt();
}
CircaFloat* Term::asFloat()
{
    return this->outputValue->asFloat();
}
CircaBool* Term::asBool()
{
    return this->outputValue->asBool();
}
CircaString* Term::asString()
{
    return this->outputValue->asString();
}
CircaFunction* Term::asFunction()
{
    return this->outputValue->asFunction();
}
CircaType* Term::asType()
{
    return this->outputValue->asType();
}

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

