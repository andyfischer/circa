#ifndef SUBROUTINE_INCLUDED
#define SUBROUTINE_INCLUDED

#include "common_headers.h"

#include "function.h"

namespace circa {

struct Subroutine : public Function
{
    Branch* branch; // <-- needs to be a Term* I think

    TermList inputPlaceholders;
    Term* outputPlaceholder;

    Subroutine();
};

Subroutine* as_subroutine(Term*);
bool is_subroutine(Term*);

void Subroutine_execute(Term* caller);

void initialize_subroutine(Branch* kernel);

} // namespace circa

#endif
