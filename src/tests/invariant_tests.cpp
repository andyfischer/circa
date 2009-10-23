// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace invariant_tests {

void test_functions()
{
    // Make sure Function type has a checkInvariants.
    test_assert(type_t::get_check_invariants_func(FUNCTION_TYPE) != NULL);

    // In a subroutine, if there is an #out term, it should be last.
    Branch branch;
    Term* func = branch.eval("def f() :: int; return 1; end");
    test_assert(check_invariants(func));

    int_value(as_branch(func), 5, "five");
    test_assert(check_invariants(func) == false);
}

void register_tests()
{
    REGISTER_TEST_CASE(invariant_tests::test_functions);
}

} // namespace invariant_tests
} // namespace circa
