#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include "common_headers.h"

#include "term_map.h"
#include "term_namespace.h"

struct Type
{
    typedef void (*AllocFunc)(Term term);
    typedef void (*DeallocFunc)(Term term);
    typedef void (*CopyFunc)(Term src, Term dest);
    typedef void (*RemapPointersFunc)(Term term, TermMap& map);

    std::string name;

    // Code
    AllocFunc alloc;
    CopyFunc copy;
    DeallocFunc dealloc;
    RemapPointersFunc remapPointers;

    // memberFunctions is a list of Functions which 'belong' to this type.
    // They are guaranteed to take an instance of this type as their first
    // argument.
    TermNamespace memberFunctions;

    Term toString;

    Type();
};

bool is_type(Term term);
Type* as_type(Term term);

void Type_alloc(Term caller);

void set_member_function(Term type, std::string name, Term function);
Term get_member_function(Term type, std::string name);

#endif
