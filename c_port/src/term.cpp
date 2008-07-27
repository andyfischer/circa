
#include <cstdio>
#include <sstream>
#include <iostream>

#include "builtins.h"
#include "branch.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "term.h"
#include "type.h"

int gNextGlobalID = 1;

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

Term::Term()
  : owningBranch(NULL),
    function(NULL),
    value(NULL),
    type(NULL),
    state(NULL),
    needsUpdate(true)
{
    globalID = gNextGlobalID++;
}

Type*
Term::getType() const
{
    return as_type(this->type);
}

const char *
Term::toString()
{
    Term* result = apply_function(this->owningBranch,
            get_global("to-string"), TermList(this));
    execute(result);
    return as_string(result).c_str();
}

int
Term::numErrors() const
{
    return (int) errors.size();
}

std::string const&
Term::getError(int index)
{
    return errors[index];
}

int& as_int(Term* t)
{
    if (t->type != BUILTIN_INT_TYPE)
        throw errors::InternalTypeError(t, BUILTIN_INT_TYPE);

    return *((int*) t->value);
}

float& as_float(Term* t)
{
    if (t->type != BUILTIN_FLOAT_TYPE)
        throw errors::InternalTypeError(t, BUILTIN_FLOAT_TYPE);

    return *((float*) t->value);
}

bool& as_bool(Term* t)
{
    if (t->type != BUILTIN_BOOL_TYPE)
        throw errors::InternalTypeError(t, BUILTIN_BOOL_TYPE);

    return *((bool*) t->value);
}

string& as_string(Term* t)
{
    if (t->type != BUILTIN_STRING_TYPE)
        throw errors::InternalTypeError(t, BUILTIN_STRING_TYPE);

    if (t->value == NULL)
        throw errors::InternalError("NULL pointer in as_string");

    return *((string*) t->value);
}

// TermList type
void TermList_alloc(Term* caller)
{
    caller->value = new TermList();
}

TermList* as_term_list(Term* term)
{
    if (term->type != BUILTIN_TERM_LIST_TYPE)
        throw errors::InternalTypeError(term, BUILTIN_TERM_LIST_TYPE);
    return (TermList*) term->value;
}

void initialize_term(Branch* kernel)
{
    BUILTIN_TERM_LIST_TYPE = quick_create_type(kernel, "TermList", TermList_alloc, NULL);
}
