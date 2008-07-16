
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
    return items[index];
}

void Term::execute()
{
    ExecContext context(this);

    CA_FUNCTION(function)->execute(&context);
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

