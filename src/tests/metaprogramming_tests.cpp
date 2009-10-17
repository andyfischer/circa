// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace metaprogramming_tests {

void test_lift_closure()
{
    Branch branch;
    Term* a = branch.eval("a = 1");
    Branch& sub = create_branch(branch);
    Term* sub_a = sub.eval("a");
    Term* sub_freeze_a = sub.eval("freeze(a)");

    test_assert(sub_a->input(0) == a);
    test_assert(as_int(sub_a) == 1);
    test_assert(sub_freeze_a->input(0) == a);
    test_assert(as_int(sub_a) == 1);

    lift_closure(sub);
    as_int(a) = 2;
    evaluate_branch(branch);

    test_assert(sub_a->input(0) == a);
    test_assert(as_int(sub_a) == 2);
    test_assert(sub_freeze_a->numInputs() == 0);
    test_assert(as_int(sub_freeze_a) == 1);
}

void register_tests()
{
    REGISTER_TEST_CASE(metaprogramming_tests::test_lift_closure);
}

} // namespace metaprogramming_tests
} // namespace circa
