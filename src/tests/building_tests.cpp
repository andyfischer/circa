// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace building_tests {

void test_create_value()
{
    Branch branch;
    Term *term = create_value(branch, INT_TYPE);
    test_assert(term->type == INT_TYPE);

    term = create_value(branch, BRANCH_TYPE);
    test_assert(get_branch_value(term) != NULL);
}

void test_create_int()
{
    Branch branch;
    Term *term = create_int(branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = create_int(branch, 154, "george");
    test_assert(term2 == branch.get("george"));
    test_assert(term2->name == "george");
    test_assert(as_int(term2) == 154);
}

void test_create_duplicate()
{
    Branch branch;

    Term* a = branch.compile("state int a = 5");

    Term* b = create_duplicate(branch, a);

    test_assert(a->function == b->function);
    test_assert(a->type == b->type);
    test_assert(a->value_type == b->value_type);
    test_assert(is_get_state(b));
}

void test_duplicate_value()
{
    Branch branch;
    Term* a = branch.eval("a = 5");
    Term* b = branch.eval("add(4,5)");

    Term* a_dup = duplicate_value(branch, a);
    test_assert(a_dup->asInt() == 5);
    test_assert(a_dup->name == "");
    test_assert(a_dup->function == VALUE_FUNC);

    Term* b_dup = duplicate_value(branch, b);
    test_assert(b_dup->asInt() == 9);
    test_assert(b_dup->name == "");
    test_assert(b_dup->function == VALUE_FUNC);
}

void test_rewrite_as_value()
{
    Branch branch;

    Term* a = create_value(branch, INT_TYPE);
    Term* b = apply(branch, ADD_FUNC, RefList(a,a));

    test_assert(branch[1] == b);

    // rewrite b
    rewrite_as_value(branch, 1, FLOAT_TYPE);

    test_assert(b->type == FLOAT_TYPE);
    test_assert(b->function == VALUE_FUNC);
    test_assert(is_value(b));

    // add new term
    rewrite_as_value(branch, 2, INT_TYPE);
    test_assert(branch.length() == 3);
    Term* c = branch[2];
    test_assert(is_value(c));
    test_assert(c->type == INT_TYPE);

    // add a new term such that we need to create NULLs
    rewrite_as_value(branch, 5, STRING_TYPE);
    test_assert(branch.length() == 6);
    test_assert(branch[3] == NULL);
    test_assert(branch[4] == NULL);
    test_assert(is_value(branch[5]));
    test_assert(branch[5]->type == STRING_TYPE);
}

void test_procure()
{
    Branch branch;
    Term* a = procure_value(branch, INT_TYPE, "a");
    procure_value(branch, INT_TYPE, "b");
    test_assert(a->type == INT_TYPE);
    test_assert(is_value(a));

    Term* a_again = procure_value(branch, INT_TYPE, "a");
    test_assert(a == a_again);

    a_again = procure_value(branch, FLOAT_TYPE, "a");
    test_assert(a == a_again);
    test_assert(a->type == FLOAT_TYPE);
    test_assert(is_value(a));
}

void test_set_input()
{
    Branch branch;

    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = print()");

    test_assert(b->numInputs() == 0);
    test_assert(a->users.length() == 0);

    set_input(b, 0, a);

    test_assert(b->numInputs() == 1);
    test_assert(b->input(0) == a);
    test_assert(a->users[0] == b);
}

void test_get_relative_input_scope()
{
    Branch branch;
    create_int(branch, 1, "a");
    Term* b = branch.compile("b = sqr(a)");
    test_equals(get_input_relative_scope(b, 0), 0);

    Branch& b1 = create_branch(branch);
    Term* c = b1.compile("c = sqr(a)");
    test_equals(get_input_relative_scope(c, 0), 1);

    Branch& b2 = create_branch(b1);
    Term* d = b2.compile("d = sqr(a)");
    test_equals(get_input_relative_scope(d, 0), 2);

    branch.compile("e = 1");
    branch.compile("if true e = 2 end");
    Term* f = branch.compile("f = sqr(e)");
    test_equals(get_input_relative_scope(f, 0), 0);

    branch.compile("g = 1");
    branch.compile("for i in [1] g = 2 end");
    Term* h = branch.compile("h = sqr(g)");
    test_equals(get_input_relative_scope(h, 0), 0);
}

void register_tests()
{
    REGISTER_TEST_CASE(building_tests::test_create_value);
    REGISTER_TEST_CASE(building_tests::test_create_int);
    REGISTER_TEST_CASE(building_tests::test_create_duplicate);
    REGISTER_TEST_CASE(building_tests::test_duplicate_value);
    REGISTER_TEST_CASE(building_tests::test_rewrite_as_value);
    REGISTER_TEST_CASE(building_tests::test_procure);
    REGISTER_TEST_CASE(building_tests::test_set_input);
    REGISTER_TEST_CASE(building_tests::test_get_relative_input_scope);
}

} // namespace building_tests
} // namespace circa
