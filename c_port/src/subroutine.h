#ifndef SUBROUTINE_INCLUDED
#define SUBROUTINE_INCLUDED

#include "common_headers.h"


struct Subroutine
{

};

Subroutine* as_subroutine(Term*);

void Subroutine_alloc(Term* term);

#endif
