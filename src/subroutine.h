#ifndef SUBROUTINE_INCLUDED
#define SUBROUTINE_INCLUDED

#include "common_headers.h"

#include "function.h"

namespace circa {

struct Subroutine : public Function
{
    Branch* branch;

    Subroutine();
};

Subroutine* as_subroutine(Term*);
bool is_subroutine(Term*);

Branch* Subroutine_openBranch(Term* caller);
void Subroutine_closeBranch(Term* caller, Branch* branch);
void Subroutine_execute(Term* caller);

void initialize_subroutine(Branch* kernel);

} // namespace circa

#endif
