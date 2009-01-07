// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "debug.h"
#include "runtime.h"
#include "term.h"

namespace circa {

bool sanity_check_term(Term* term)
{
    if (is_bad_pointer(term))
        return false;

    if(!term->function->users.contains(term))
        return false;

    for (unsigned int i=0; i < term->inputs.count(); i++) {
        if(!term->inputs[i]->users.contains(term))
            return false;
    }

    for (unsigned int i=0; i < term->users.count(); i++) {
        if (!is_actually_using(term->users[i], term))
            return false;
    }

    return true;
}

} // namespace circa
