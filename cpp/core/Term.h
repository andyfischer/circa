#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "CommonHeaders.h"

#include "TermList.h"

struct Term
{
    Term() :
        data(NULL),
        function(NULL),
        needs_update(true),
        debug_name("")
    {}

    void* data;
    TermList inputs;
    Term* function;
    TermList users;
    bool needs_update;
    string debug_name;

    // Accessors
    Term* input(int index);
    Term* get_type() const;
    string to_string() const;
    string debug_identifier() const;

    void evaluate();
};

#endif
