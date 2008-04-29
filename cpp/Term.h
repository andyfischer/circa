#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "CommonHeaders.h"

#include "TermList.h"

struct Term
{
    Term() :
        _data(NULL),
        _function(NULL)
    {}

    void* _data;
    TermList _inputs;
    Term* _function;
};

#endif
