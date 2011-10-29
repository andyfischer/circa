// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>
#include "update_cascades.h"

namespace circa {
namespace building_tests {

void test_create_value()
{
    Branch branch;
    Term *term = create_value(&branch, &INT_T);
    test_assert(term->type == &INT_T);
}

void test_create_int()
{
    Branch branch;
    Term *term = create_int(&branch, -2);
    test_assert(as_int(term) == -2);

    Term *term2 = create_int(&branch, 154, "george");
    test_assert(term2 == branch.get("george"));
    test_assert(term2->name == "george");
    test_assert(as_int(term2) == 154);
}

void test_create_duplicate()
{
    Branch branch;

    Term* a = branch.compile("state int a = 5");

    Term* b = create_duplicate(&branch, a);

    test_assert(a->function == b->function);
    test_assert(a->type == b->type);
    test_assert(a->value_type == b->value_type);
    test_assert(is_get_state(b));
}

void test_duplicate_value()
{
    Branch branch;
    Term* a = branch.compile("a = 5");

    Term* a_dup = duplicate_value(&branch, a);
    test_assert(a_dup->asInt() == 5);
    test_assert(a_dup->name == "");
    test_assert(a_dup->function == VALUE_FUNC);
}

void test_rewrite_as_value()
{
    Branch branch;

    Term* a = create_value(&branch, &INT_T);
    Term* b = apply(&branch, ADD_FUNC, TermList(a,a));

    test_assert(branch[1] == b);

    // rewrite b
    rewrite_as_value(&branch, 1, &FLOAT_T);

    test_assert(b->type == &FLOAT_T);
    test_assert(b->function == VALUE_FUNC);
    test_assert(is_value(b));

    // add new term
    rewrite_as_value(&branch, 2, &INT_T);
    test_assert(branch.length() == 3);
    Term* c = branch[2];
    test_assert(is_value(c));
    test_assert(c->type == &INT_T);

    // add a new term such that we need to create NULLs
    rewrite_as_value(&branch, 5, &STRING_T);
    test_assert(branch.length() == 6);
    test_assert(branch[3] == NULL);
    test_assert(branch[4] == NULL);
    test_assert(is_value(branch[5]));
    test_assert(branch[5]->type == &STRING_T);
}

void test_procure()
{
    Branch branch;
    Term* a = procure_value(&branch, &INT_T, "a");
    procure_value(&branch, &INT_T, "b");
    test_assert(a->type == &INT_T);
    test_assert(is_value(a));

    Term* a_again = procure_value(&branch, &INT_T, "a");
    test_assert(a == a_again);

    a_again = procure_value(&branch, &FLOAT_T, "a");
    test_assert(a == a_again);
    test_assert(a->type == &FLOAT_T);
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

void test_finish_branch_is_at_end()
{
    Branch branch;
    branch.compile("a = 1 + 2");
    test_assert(branch[branch.length()-1]->function != FINISH_MINOR_BRANCH_FUNC);
    apply(&branch, FINISH_MINOR_BRANCH_FUNC, TermList());
    test_assert(branch[branch.length()-1]->function == FINISH_MINOR_BRANCH_FUNC);
    branch.compile("b = 3 / 4");
    test_assert(branch[branch.length()-1]->function == FINISH_MINOR_BRANCH_FUNC);
}

void test_erase_term()
{
    Branch branch;

    // Erase an input that's being used
    Term* a = branch.compile("a = 1");
    Term* b = branch.compile("b = add(a a)");
    test_assert(b->input(0) == a);

    erase_term(a);
    test_assert(b->input(0) == NULL);

    // Erase a function that's being used
    Term* f = branch.compile("def f() {}");
    Term* f_call = branch.compile("f()");
    test_assert(f_call->function == f);

    erase_term(f);
    test_assert(f_call->function == NULL);
}

void test_repair_broken_links()
{
    #if 0
    // TEST_DISABLED

    Branch branch;
    Term* br = branch.compile("br = { a = 1 }");
    Term* a = br->contents()["a"];
    Term* b = branch.compile("add_call = add()");
    set_input(b, 0, a);

    clear_branch(nested_contents(br));

    test_assert(b->input(0) == NULL);

    finish_update_cascade(branch);

    test_assert(b->input(0) == NULL);

    clear_branch(&branch);
    br = branch.compile("br = { a = 1 }");
    a = br->nestedContents["a"];
    b = branch.compile("add()");
    set_input(b, 0, a);

    clear_branch(&br->nestedContents);
    br->nestedContents.compile("something_else = 1, a = 2");

    // bit of a hack:
    br->setBoolProp("exposesNames", true);

    Term* new_a = br->nestedContents["a"];
    finish_update_cascade(branch);

    test_assert(a != new_a);
    test_assert(b->input(0) == new_a);
    #endif
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
    REGISTER_TEST_CASE(building_tests::test_finish_branch_is_at_end);
    REGISTER_TEST_CASE(building_tests::test_erase_term);
    REGISTER_TEST_CASE(building_tests::test_repair_broken_links);
}

} // namespace building_tests
} // namespace circa
