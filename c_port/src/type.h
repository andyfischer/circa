#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include <string>

#include "term_namespace.h"

struct Term;

struct Type
{
    typedef void (*AllocFunc)(Term* term);
    typedef void (*DeallocFunc)(Term* term);
    typedef void (*CopyFunc)(Term* src, Term* dest);

    std::string name;

    // Code
    AllocFunc alloc;
    CopyFunc copy;
    DeallocFunc dealloc;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take a term of our type as their first
    // argument.
    TermNamespace memberFunctions;

    Term* toString;

    Type();
};

extern "C" {

bool is_type(Term* term);
Type* as_type(Term* term);
void Type_alloc(Term* caller);
void Type_setName(Term* term, const char* value);
void Type_setAllocFunc(Term* term, Type::AllocFunc allocFunc);

}

#endif
