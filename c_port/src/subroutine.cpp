
#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "operations.h"
#include "subroutine.h"
#include "term.h"

Subroutine::Subroutine()
  : branch(NULL),
    outputPlaceholder(NULL)
{
}

Subroutine* as_subroutine(Term* term)
{
    if (term->type != BUILTIN_SUBROUTINE_TYPE)
        throw errors::InternalTypeError(term, BUILTIN_SUBROUTINE_TYPE);

    return (Subroutine*) term->value;
}

void Subroutine_alloc(Term* term)
{
    term->value = new Subroutine();
    as_subroutine(term)->branch = new Branch();
}

void Subroutine_dealloc(Term* term)
{
    delete as_subroutine(term)->branch;
    delete as_subroutine(term);
}

void Subroutine_execute(Term* caller)
{
    Subroutine* sub = as_subroutine(caller->function);

    // Copy inputs to input placeholders
    for (int inputIndex=0; inputIndex < sub->inputTypes.count(); inputIndex++) {
        copy_term(caller->inputs[inputIndex], sub->inputPlaceholders[inputIndex]);
    }

    // Execute every term of ours
    execute_branch(sub->branch);

    // Copy output to output placeholder
    if (sub->outputPlaceholder != NULL)
        copy_term(sub->outputPlaceholder, caller);
}

void initialize_subroutine(Branch* kernel)
{
    BUILTIN_SUBROUTINE_TYPE = quick_create_type(KERNEL, "Subroutine", Subroutine_alloc, NULL);
}
