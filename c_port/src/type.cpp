
#include "builtins.h"
#include "common_headers.h"
#include "term.h"
#include "type.h"


Type::Type()
{
}

Type* as_type(Term* term)
{
    // todo: type check
    return (Type*) term->value;
}

void Type_alloc(Term* caller)
{
    caller->value = new Type();
}

void Type_setName(Term* term, const char* value)
{
    as_type(term)->name = value;
}

void Type_setAllocFunc(Term* term, void(*allocFunc)(Term*))
{
    as_type(term)->alloc = allocFunc;
}
