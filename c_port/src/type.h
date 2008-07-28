#ifndef CIRCA__TYPE__INCLUDED
#define CIRCA__TYPE__INCLUDED

#include <string>

struct Term;

struct Type
{
    typedef void (*AllocFunc)(Term* term);
    typedef void (*CopyFunc)(Term* src, Term* dest);

    std::string name;

    Type();
    AllocFunc alloc;
    CopyFunc copy;

    Term* toString;
};

extern "C" {

bool is_type(Term* term);
Type* as_type(Term* term);
void Type_alloc(Term* caller);
void Type_setName(Term* term, const char* value);
void Type_setAllocFunc(Term* term, Type::AllocFunc allocFunc);

}

#endif
