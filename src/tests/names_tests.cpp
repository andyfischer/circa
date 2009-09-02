// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include <circa.h>

namespace circa {
namespace names_tests {

void test_find_named()
{
    Branch branch;
    Term* a = int_value(branch, 1, "a");

    test_assert(find_named(branch, "a") == a);
    test_assert(find_named(branch, "b") == NULL);

    Branch& sub = create_branch(branch, "sub");
    test_assert(find_named(sub, "a") == a);
}

void test_get_dot_separated_name()
{
    Branch branch;
    branch.eval("namespace a; b = 1; end");

    test_assert(get_dot_separated_name(branch, "a.b") != NULL);
    test_assert(get_dot_separated_name(branch, "a.b")->asInt() == 1);
    test_assert(get_dot_separated_name(branch, "a.c") == NULL);
    test_assert(get_dot_separated_name(branch, "b.a") == NULL);
}

void register_tests()
{
    REGISTER_TEST_CASE(names_tests::test_find_named);
    REGISTER_TEST_CASE(names_tests::test_get_dot_separated_name);
}

} // namespace names_tests
} // namespace circa
