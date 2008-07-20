
#include <cstdio>
#include <sstream>
#include <iostream>

#include "builtins.h"
#include "branch.h"
#include "errors.h"
#include "function.h"
#include "term.h"


void TermList::setAt(int index, Term* term)
{
    // Make sure there are enough blank elements in the list
    while (items.size() <= index) {
        items.push_back(NULL);
    }

    items[index] = term;
}
Term* TermList::operator[](int index)
{
    if (index >= items.size())
        return NULL;
    return items[index];
}

void Term::execute()
{
    if (this->function == NULL)
        throw errors::InternalError("function term is NULL");

    Function* func = as_function(this->function);

    if (func == NULL)
        throw errors::InternalError("function is NULL");

    if (func->execute == NULL) {
        std::cout << "warning: no evaluate function for " << func->name << std::endl;
        return;
    }
        
    func->execute(this);
}

int& as_int(Term* t)
{
    if (t->type != BUILTIN_INT_TYPE)
        throw errors::TypeError();

    return *((int*) t->value);
}

float& as_float(Term* t)
{
    if (t->type != BUILTIN_FLOAT_TYPE)
        throw errors::TypeError();

    return *((float*) t->value);
}

bool& as_bool(Term* t)
{
    if (t->type != BUILTIN_BOOL_TYPE)
        throw errors::TypeError();

    return *((bool*) t->value);
}

string& as_string(Term* t)
{
    if (t->type != BUILTIN_STRING_TYPE)
        throw errors::TypeError();

    if (t->value == NULL)
        throw errors::InternalError("NULL pointer in as_string");

    return *((string*) t->value);
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

