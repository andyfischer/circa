
#include "builtins.h"
#include "errors.h"
#include "subroutine.h"
#include "term.h"

Subroutine* as_subroutine(Term* term)
{
    if (term->type != BUILTIN_SUBROUTINE_TYPE)
        throw errors::InternalTypeError(term, BUILTIN_SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

void Subroutine_alloc(Term* type, Term* term)
{
    term->value = new Subroutine;
}
