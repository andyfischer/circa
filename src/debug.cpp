// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

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

void sanity_check_term(Term* term)
{
    assert_good_pointer(term);

#if TRACK_USERS
    for (unsigned int i=0; i < term->inputs.count(); i++) {
        assert(term->inputs[i]->users.contains(term));
    }

    for (unsigned int i=0; i < term->users.count(); i++) {
        assert(is_actually_using(term->users[i], term));
    }
#endif
}

void sanity_check_the_world()
{
    for (int i=0; i < KERNEL->numTerms(); i++) {
        Term* term = KERNEL->get(i);
        sanity_check_term(term);
    }
}

} // namespace circa
