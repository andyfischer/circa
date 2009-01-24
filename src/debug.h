// Copyright 2008 Andrew Fischer

#ifndef CIRCA_DEBUG_INCLUDED
#define CIRCA_DEBUG_INCLUDED

namespace circa {

#define DEBUG_CHECK_FOR_BAD_POINTERS true
#define DEBUG_NEVER_DELETE_TERMS true

#if DEBUG_CHECK_FOR_BAD_POINTERS
extern std::set<Term*> DEBUG_GOOD_POINTER_SET;
#endif

bool is_bad_pointer(Term* term);
void assert_good_pointer(Term* term);

// Perform a bunch of checks to see if this term is healthy, and all its related
// data is consistent.
bool sanity_check_term(Term* term);

} // namespace circa

#endif
