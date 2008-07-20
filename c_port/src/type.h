#ifndef __TYPE_INCLUDED__
#define __TYPE_INCLUDED__

#include <string>

struct Term;

struct Type
{
    string name;

    Type();
    void (*alloc)(Term*);
    Term* toString;
};

extern "C" {

Type* as_type(Term* term);
void Type_alloc(Term* caller);
void Type_setName(Term* term, const char* value);
void Type_setAllocFunc(Term* term, void(*allocFunc)(Term*));

}

#endif
