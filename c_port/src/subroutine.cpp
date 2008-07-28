
#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"
#include "term.h"

Subroutine* as_subroutine(Term* term)
{
    if (term->type != BUILTIN_SUBROUTINE_TYPE)
        throw errors::InternalTypeError(term, BUILTIN_SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

void Subroutine_alloc(Term* term)
{
    term->value = new Subroutine;
}

void initialize_subroutine(Branch* kernel)
{
    BUILTIN_SUBROUTINE_TYPE = quick_create_type(KERNEL, "Subroutine", Subroutine_alloc, NULL);
}
