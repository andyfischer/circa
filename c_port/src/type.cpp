
#include "bootstrap.h"
#include "common_headers.h"
#include "object.h"
#include "term.h"
#include "type.h"

#define CA_TYPE(t) ((CircaType*)t->outputValue)

CircaType::CircaType()
{
    _type = BUILTIN_TYPE_TYPE;
}

CircaObject* CaType_alloc(Term*)
{
    return new CircaType();
}

void CaType_setName(Term* term, const char* value)
{
    CA_TYPE(term)->name = value;
}

void CaType_setAllocFunc(Term* term, CircaObject*(*allocFunc)(Term*))
{
    CA_TYPE(term)->alloc = allocFunc;
}
