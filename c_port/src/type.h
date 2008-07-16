#ifndef __TYPE_INCLUDED__
#define __TYPE_INCLUDED__

#include <string>

#include "object.h"

#define CA_TYPE(t) ((CircaType*)t->outputValue)

struct Term;

struct CircaType : public CircaObject
{
    string name;

    CircaType();
    CircaObject* (*alloc)(Term*);
    Term* toString;
};

extern "C" {

CircaObject* CaType_alloc(Term*);
void CaType_setName(Term* term, const char* value);
void CaType_setAllocFunc(Term* term, CircaObject*(*allocFunc)(Term*));

}

#endif
