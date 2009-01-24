// Copyright 2008 Andrew Fischer

#ifndef CIRCA_DEBUG_INCLUDED
#define CIRCA_DEBUG_INCLUDED

namespace circa {

// This flag enables a global map of 'good' Term pointers. When a Term
// is deleted, it's address is no longer 'good'. Then a call to
// assert_good_pointer will fail. This feature has a performance cost.
#define DEBUG_CHECK_FOR_BAD_POINTERS true

// Enabling this flag causes us to never actually delete Term objects.
// This removes the possibility that a bad pointer will mistakenly be
// deemed a good pointer, just because a new Term occupies the same
// memory that a previous Term did.
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
