#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "CommonHeaders.h"

#include "TermList.h"

struct Term
{
    Term() :
        data(NULL),
        function(NULL),
        debug_name("undefined")
    {}

    void* data;
    TermList inputs;
    Term* function;
    string debug_name;

    Term* get_type() const;
    string to_string() const;
};

#endif
