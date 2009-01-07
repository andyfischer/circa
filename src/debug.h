// Copyright 2008 Andrew Fischer

#ifndef CIRCA_DEBUG_INCLUDED
#define CIRCA_DEBUG_INCLUDED

namespace circa {

// Perform a bunch of checks to see if this term is healthy, and all its related
// data is consistent.
bool sanity_check_term(Term* term);

} // namespace circa

#endif
