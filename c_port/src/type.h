#ifndef __TYPE_INCLUDED__
#define __TYPE_INCLUDED__

#include <string>

#include "object.h"

using std::string;

struct CircaType : public CircaObject
{
    string name;

    CircaObject* (*alloc)(Term*);
};

CircaType* CaType_alloc(Term*);
void CaType_setName(Term* term, const char* value);
void CaType_setAllocFunc(Term* term, void* allocFunc);

#endif
