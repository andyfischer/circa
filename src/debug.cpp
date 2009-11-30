// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include "circa.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_ERROR_OCCURRED = false;

#if DEBUG_CHECK_FOR_BAD_POINTERS
std::set<Term*> *DEBUG_GOOD_POINTER_SET = NULL;
#endif

void register_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    if (DEBUG_GOOD_POINTER_SET == NULL)
        DEBUG_GOOD_POINTER_SET = new std::set<Term*>();
    DEBUG_GOOD_POINTER_SET->insert(term);
#endif
}

void unregister_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    DEBUG_GOOD_POINTER_SET->erase(term);
#endif
}

bool is_bad_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    if (DEBUG_GOOD_POINTER_SET == NULL)
        DEBUG_GOOD_POINTER_SET = new std::set<Term*>();
    return DEBUG_GOOD_POINTER_SET->find(term) == DEBUG_GOOD_POINTER_SET->end();
#else
    return false;
#endif
}

void assert_good_pointer(Term* term)
{
#if DEBUG_CHECK_FOR_BAD_POINTERS
    if (is_bad_pointer(term))
        throw std::runtime_error("assert_good_pointer failed (bad term pointer)");
#endif
}

void dump_branch(Branch& branch)
{
    std::cout << print_branch_raw(branch);
}

void dump_branch_with_props(Branch& branch)
{
    std::cout << print_branch_raw_with_properties(branch);
}

} // namespace circa
