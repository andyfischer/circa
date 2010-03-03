// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_ERROR_OCCURRED = false;

#if DEBUG_CHECK_FOR_BAD_POINTERS
std::set<Term*> DEBUG_GOOD_POINTER_SET;
#endif

void register_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    assert(DEBUG_GOOD_POINTER_SET.find(term) == DEBUG_GOOD_POINTER_SET.end());
    DEBUG_GOOD_POINTER_SET.insert(term);
#endif
}

void unregister_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    assert(DEBUG_GOOD_POINTER_SET.find(term) != DEBUG_GOOD_POINTER_SET.end());
    DEBUG_GOOD_POINTER_SET.erase(term);
#endif
}

bool is_bad_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    return DEBUG_GOOD_POINTER_SET.find(term) == DEBUG_GOOD_POINTER_SET.end();
#else
    return false;
#endif
}

void assert_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    assert(DEBUG_GOOD_POINTER_SET.find(term) != DEBUG_GOOD_POINTER_SET.end());
#endif
}

void dump_branch(Branch& branch)
{
    print_branch_raw(std::cout, branch);
}

void dump_branch_with_props(Branch& branch)
{
    print_branch_raw_with_properties(std::cout, branch);
}

} // namespace circa
