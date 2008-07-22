
#include "common_headers.h"

#include "builtins.h"
#include "errors.h"
#include "term.h"
#include "type.h"

Type::Type()
  : name("undefined"),
    alloc(NULL),
    copy(NULL),
    toString(NULL)
{
}

bool is_type(Term* term)
{
    return ((term->type == BUILTIN_TYPE_TYPE)
            || (term->type == BUILTIN_STRUCT_DEFINITION_TYPE));
}

Type* as_type(Term* term)
{
    if (!is_type(term))
        throw errors::InternalTypeError(term, BUILTIN_TYPE_TYPE);

    return (Type*) term->value;
}

void Type_alloc(Term* type, Term* caller)
{
    caller->value = new Type();
}

void Type_setName(Term* term, const char* value)
{
    as_type(term)->name = value;
}

void Type_setAllocFunc(Term* term, Type::AllocFunc allocFunc)
{
    as_type(term)->alloc = allocFunc;
}
