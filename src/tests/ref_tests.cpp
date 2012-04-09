// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa_internal.h"

namespace circa {
namespace ref_tests {

void test_refs_are_destruction_safe()
{
    Branch branch;
    Term* a = branch.compile("a = 1");
    caValue ref;
    set_ref(&ref, a);
    test_assert(as_ref(&ref) == a);
    clear_branch(&branch);
    test_assert(as_ref(&ref) == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(ref_tests::test_refs_are_destruction_safe);
}

} // namespace ref_tests
} // namespace circa
