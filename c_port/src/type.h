#ifndef __TYPE_INCLUDED__
#define __TYPE_INCLUDED__

#include <string>

#include "object.h"

struct Term;

struct CircaType : public CircaObject
{
    string name;

    CircaObject* (*alloc)(Term*);
};

extern "C" {

CircaObject* CaType_alloc(Term*);
void CaType_setName(Term* term, const char* value);
void CaType_setAllocFunc(Term* term, CircaObject*(*allocFunc)(Term*));

}

#endif
