#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "CommonHeaders.h"

#include "TermList.h"

struct Term
{
    Term() :
        data(NULL),
        function(NULL)
    {}

    void* data;
    TermList inputs;
    Term* function;
};

#endif
