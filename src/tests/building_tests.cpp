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

void test_rewrite_as_value()
{
    Branch branch;

    Term* a = create_value(&branch, INT_TYPE);
    Term* b = apply(&branch, ADD_FUNC, RefList(a,a));

    test_assert(branch[1] == b);

    // rewrite b
    rewrite_as_value(branch, 1, FLOAT_TYPE);

    test_assert(b->type == FLOAT_TYPE);
    test_assert(b->function == get_value_function(FLOAT_TYPE));
    test_assert(is_value(b));

    // add new term
    rewrite_as_value(branch, 2, INT_TYPE);
    test_assert(branch.numTerms() == 3);
    Term* c = branch[2];
    test_assert(is_value(c));
    test_assert(c->type == INT_TYPE);

    // add a new term such that we need to create NULLs
    rewrite_as_value(branch, 5, STRING_TYPE);
    test_assert(branch.numTerms() == 6);
    test_assert(branch[3] == NULL);
    test_assert(branch[4] == NULL);
    test_assert(is_value(branch[5]));
    test_assert(branch[5]->type == STRING_TYPE);
}

void register_tests()
{
    REGISTER_TEST_CASE(building_tests::test_create_value);
    REGISTER_TEST_CASE(building_tests::test_int_value);
    REGISTER_TEST_CASE(building_tests::test_create_duplicate);
    REGISTER_TEST_CASE(building_tests::test_rewrite_as_value);
}

} // namespace building_tests
} // namespace circa
