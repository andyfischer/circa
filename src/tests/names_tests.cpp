// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

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

void test_name_is_reachable_from()
{
    Branch branch;

    Term* a = int_value(branch, 5, "a");

    test_assert(name_is_reachable_from(a, branch));

    Branch& sub_1 = create_branch(branch, "sub_1");
    test_assert(name_is_reachable_from(a, sub_1));

    Branch& sub_2 = create_branch(sub_1, "sub_2");
    test_assert(name_is_reachable_from(a, sub_2));
}

void test_get_relative_name()
{
    Branch branch;
    Term* a = int_value(branch, 5, "A");
    test_assert(get_relative_name(branch, a) == "A");

    Branch& ns = create_namespace(branch, "ns");
    Term* b = int_value(ns, 5, "B");

    test_assert(get_relative_name(ns, b) == "B");
    test_equals(get_relative_name(branch, b), "ns.B");
}

void register_tests()
{
    REGISTER_TEST_CASE(names_tests::test_find_named);
    REGISTER_TEST_CASE(names_tests::test_get_dot_separated_name);
    REGISTER_TEST_CASE(names_tests::test_name_is_reachable_from);
    REGISTER_TEST_CASE(names_tests::test_get_relative_name);
}

} // namespace names_tests
} // namespace circa
