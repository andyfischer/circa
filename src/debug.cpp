// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "debug.h"
#include "runtime.h"
#include "term.h"

namespace circa {

#if DEBUG_CHECK_FOR_BAD_POINTERS
std::set<Term*> DEBUG_GOOD_POINTER_SET;
#endif

bool is_bad_pointer(Term* term)
{
    return DEBUG_GOOD_POINTER_SET.find(term) == DEBUG_GOOD_POINTER_SET.end();
}

void assert_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    if (is_bad_pointer(term))
        throw std::runtime_error("assert_good_pointer failed (bad term pointer)");
#endif
}

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
