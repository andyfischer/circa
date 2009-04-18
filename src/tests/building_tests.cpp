// Copyright 2009 Andrew Fischer

#include <circa.h>

namespace circa {
namespace building_tests {

void test_create_value()
{
    Branch branch;
    Term *term = create_value(&branch, INT_TYPE);
    test_assert(term->type == INT_TYPE);
    test_assert(term->value != NULL);

    term = create_value(&branch, BRANCH_TYPE);
    test_assert(term->value != NULL);
    // test_assert(as_branch(term).owningTerm == term);
}

void test_int_value()
{
    Branch branch;
    Term *term = int_value(&branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = int_value(&branch, 154, "george");
    test_assert(term2 == branch.getNamed("george"));
    test_assert(term2->name == "george");
    test_assert(as_int(term2) == 154);
}

void test_create_duplicate()
{
    Branch branch;

    Term* a = branch.eval("state int a = 5");

    Term* b = create_duplicate(&branch, a);

    test_assert(a->function == b->function);
    test_assert(a->type == b->type);
    test_assert(is_stateful(b));
}

void register_tests()
{
    REGISTER_TEST_CASE(building_tests::test_create_value);
    REGISTER_TEST_CASE(building_tests::test_int_value);
    REGISTER_TEST_CASE(building_tests::test_create_duplicate);
}

} // namespace building_tests
} // namespace circa
