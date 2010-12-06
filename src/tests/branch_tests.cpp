// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace branch_tests {

void test_insert()
{
    Branch branch;

    test_assert(branch_check_invariants_print_result(branch, std::cout)); // sanity check

    Term* a = create_void(branch);

    test_assert(branch_check_invariants_print_result(branch, std::cout));

    Term* b = alloc_term();
    test_assert(b->value_type != NULL);

    test_assert(branch.length() == 1);
    test_assert(branch[0] == a);
    
    test_assert(branch_check_invariants_print_result(branch, std::cout));

    branch.insert(0, b);

    test_assert(branch_check_invariants_print_result(branch, std::cout));

    test_assert(branch.length() == 2);
    test_assert(branch[0] == b);
    test_assert(branch[1] == a);

    test_assert(branch_check_invariants_print_result(branch, std::cout));

    Term* c = alloc_term();
    branch.insert(2, c);
    test_assert(branch.length() == 3);
    test_assert(branch[0] == b);
    test_assert(branch[1] == a);
    test_assert(branch[2] == c);

    test_assert(branch_check_invariants_print_result(branch, std::cout));

    Term* d = alloc_term();
    branch.insert(1, d);
    test_assert(branch.length() == 4);
    test_assert(branch[0] == b);
    test_assert(branch[1] == d);
    test_assert(branch[2] == a);
    test_assert(branch[3] == c);

    test_assert(branch_check_invariants_print_result(branch, std::cout));
}

void test_check_invariants()
{
    Branch branch;
    std::stringstream output;
    Term* t = branch.appendNew();
    test_assert(branch_check_invariants_print_result(branch, output));
    t->owningBranch = NULL;
    test_assert(!branch_check_invariants_print_result(branch, output));

    branch.clear();
    t = branch.appendNew();
    test_assert(branch_check_invariants_print_result(branch, output));
    t->index = 5;
    test_assert(!branch_check_invariants_print_result(branch, output));
}

void test_setNull()
{
    Branch branch;
    Term* t = branch.appendNew();
    branch.bindName(t, "a");
    test_assert(branch.contains("a"));
    test_assert(t->owningBranch == &branch);
    test_assert(branch[0] == t);

    branch.setNull(0);
    test_assert(!branch.contains("a"));
    test_assert(branch[0] == NULL);
}

void test_remove()
{
    Branch branch;

    create_value(branch, INT_TYPE, "a");

    test_assert(branch.length() == 1);
    test_assert(branch.contains("a"));

    branch.remove(0);

    test_assert(branch.length() == 0);
    test_assert(!branch.contains("a"));
    test_assert(branch);
}

void test_removeNulls()
{
    Branch branch;

    branch.append(NULL);
    test_assert(branch.length() == 1);
    branch.removeNulls();
    test_assert(branch.length() == 0);

    branch.append(NULL);
    branch.append(NULL);
    branch.append(NULL);
    branch.append(NULL);
    test_assert(branch.length() == 4);
    branch.removeNulls();
    test_assert(branch.length() == 0);

    Term* a = branch.appendNew();
    branch.append(NULL);
    Term* b = branch.appendNew();
    branch.append(NULL);
    Term* c = branch.appendNew();
    branch.append(NULL);

    test_assert(branch.length() == 6);
    branch.removeNulls();
    test_assert(branch.length() == 3);
    test_assert(branch[0] == a);
    test_assert(branch[1] == b);
    test_assert(branch[2] == c);

    test_assert(branch_check_invariants_print_result(branch, std::cout));
}

void test_duplicate()
{
    Branch original;
    Term* term1 = create_int(original, 5);
    Term* term2 = create_string(original, "yarn");
    original.bindName(term1, "term1");
    original.bindName(term2, "term two");

    Branch duplicate;
    duplicate_branch(original, duplicate);

    Term* term1_duplicate = duplicate["term1"];
    test_assert(term1_duplicate != NULL);

    Term* term2_duplicate = duplicate["term two"];

    test_assert(as_int(term1_duplicate) == 5);
    test_assert(as_string(term2_duplicate) == "yarn");

    // make sure 'duplicate' uses different terms
    set_int(term1, 8);
    test_assert(as_int(term1_duplicate) == 5);

    test_assert(term1);
    test_assert(term2);
    test_assert(term1_duplicate);
    test_assert(term2_duplicate);

    test_assert(original);
    test_assert(duplicate);
}

void test_duplicate_nested()
{
    Branch branch;
    branch.compile("a = 1.0");
    Branch& inner = branch.compile("inner = branch()")->nestedContents;
    inner.compile("i = 2.0");
    inner.compile("j = add(a,i)");

    Branch dupe;
    duplicate_branch(branch, dupe);

    Term* inner_i = dupe["inner"]->nestedContents["i"];
    Term* inner_j = dupe["inner"]->nestedContents["j"];

    test_assert(inner_i != NULL);
    test_assert(inner_j != NULL);

    test_assert(dupe["a"]->asFloat() == 1.0);
    test_assert(inner_i->asFloat() == 2.0);
    test_assert(inner_j->input(0) != branch["a"]);
    test_assert(inner_j->input(0) != NULL);
    //test_assert(inner_j->input(0) == dupe["a"]);
    //test_assert(inner_j->input(0)->input(0) == dupe["a"]);
    test_assert(inner_j->input(1) == inner_i);

    test_assert(branch);
    test_assert(dupe);
}

void test_duplicate_nested_dont_make_extra_terms()
{
    // this test case looks for a bug where nested branches have
    // too many terms after duplication
    Branch orig;
    Branch& inner = orig.compile("inner = branch()")->nestedContents;
    inner.compile("i = 2");

    Branch dupe;
    duplicate_branch(orig, dupe);

    test_assert(dupe["inner"]->nestedContents.length() == 1);

    test_assert(orig);
    test_assert(dupe);
}

void test_duplicate_subroutine()
{
    Branch branch;

    Term* func = branch.compile("def func()\na = 1\nend");

    // sanity check:
    test_assert(function_t::get_name(func) == "func");
    test_assert(branch.contains("func"));

    Branch dupe;
    duplicate_branch(branch, dupe);

    test_assert(dupe.contains("func"));

    Term* dupedFunc = dupe["func"];
    test_assert(function_t::get_name(dupedFunc) == "func");

    test_assert(func->nestedContents.length() == dupedFunc->nestedContents.length());
    test_assert(func->nestedContents[1]->function == dupedFunc->nestedContents[1]->function);
    test_assert(func->nestedContents[1]->type == dupedFunc->nestedContents[1]->type);
    test_assert(func->nestedContents[1]->asInt() == dupedFunc->nestedContents[1]->asInt());
    test_assert(dupedFunc->nestedContents[1] == dupedFunc->nestedContents["a"]);

    test_assert(branch);
    test_assert(dupe);
}

void test_duplicate_get_field_by_name()
{
    Branch branch;
    branch.compile("type mytype { int f }");
    branch.compile("v = mytype()");
    Term* b = branch.compile("b = v.f");

    test_assert(b->function == GET_FIELD_FUNC);

    Branch dupe;
    duplicate_branch(branch, dupe);

    b = dupe["b"];

    test_assert(b->function == GET_FIELD_FUNC);

    test_assert(branch);
    test_assert(dupe);
}

void test_duplicate_destination_has_different_type()
{
    Branch source, dest;
    TaggedValue* a = source.eval("a = any()");
    TaggedValue* p = source.eval("p = Point()");
    copy(p,a);
    // This once tripped an assert:
    duplicate_branch(source, dest);
}

void find_name_in_outer_branch()
{
    Branch branch;
    Branch& nested = create_branch(branch);

    Term* a = nested.compile("a = 1");

    Branch& nested2 = create_branch(nested);

    test_assert(find_named(nested2, "a") == a);

    test_assert(branch);
}

void test_get_source_file_location()
{
    Branch branch;
    create_string(branch, "c:/a/b/something.ca", "attr:source-file");
    test_equals(get_source_file_location(branch), "c:/a/b");
    test_assert(branch);
}

void test_shorten()
{
    Branch branch;
    create_int(branch, 5);
    create_int(branch, 5);
    create_int(branch, 5);

    test_assert(branch.length() == 3);

    branch.shorten(3);

    test_assert(branch.length() == 3);

    branch.shorten(1);

    test_assert(branch.length() == 1);

    branch.shorten(3);

    test_assert(branch.length() == 1);

    branch.shorten(0);

    test_assert(branch.length() == 0);

    test_assert(branch);
}

void test_move()
{
    Branch branch;
    Term* one = create_int(branch, 1, "1");
    Term* two = create_int(branch, 2, "2");
    Term* three = create_int(branch, 3, "3");
    Term* four = create_int(branch, 4, "4");

    test_assert(branch.length() == 4);
    test_assert(one->index == 0);
    test_assert(two->index == 1);
    test_assert(three->index == 2);
    test_assert(four->index == 3);

    branch.move(one, 3);
    test_assert(branch.length() == 4);
    test_assert(one->index == 3);
    test_assert(two->index == 0);
    test_assert(three->index == 1);
    test_assert(four->index == 2);

    branch.move(one, 0);
    test_assert(branch.length() == 4);
    test_assert(one->index == 0);
    test_assert(two->index == 1);
    test_assert(three->index == 2);
    test_assert(four->index == 3);

    branch.move(two, 1);
    test_assert(branch.length() == 4);
    test_assert(one->index == 0);
    test_assert(two->index == 1);
    test_assert(three->index == 2);
    test_assert(four->index == 3);

    branch.move(two, 2);
    test_assert(branch.length() == 4);
    test_assert(one->index == 0);
    test_assert(two->index == 2);
    test_assert(three->index == 1);
    test_assert(four->index == 3);
    
    branch.move(four, 2);
    test_assert(branch.length() == 4);
    test_assert(one->index == 0);
    test_assert(two->index == 3);
    test_assert(three->index == 1);
    test_assert(four->index == 2);
}

void test_locals_stack()
{
    Branch branch;
    EvalContext context;

    start_using(branch);

    Term* a = branch.compile("a = 5");
    Term* b = branch.compile("b = add(a,a)");

    for (int i=0; i < branch.length(); i++)
        evaluate_single_term(&context, branch[i]);

    test_equals(get_local(b)->asInt(), 10);

    start_using(branch);

    set_int(a, 11);
    for (int i=0; i < branch.length(); i++)
        evaluate_single_term(&context, branch[i]);

    test_equals(get_local(b)->asInt(), 22);
    finish_using(branch);

    test_equals(get_local(b)->asInt(), 10);
}

void register_tests()
{
    REGISTER_TEST_CASE(branch_tests::test_insert);
    REGISTER_TEST_CASE(branch_tests::test_check_invariants);
    REGISTER_TEST_CASE(branch_tests::test_setNull);
    REGISTER_TEST_CASE(branch_tests::test_remove);
    REGISTER_TEST_CASE(branch_tests::test_removeNulls);
    REGISTER_TEST_CASE(branch_tests::test_duplicate);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_nested);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_nested_dont_make_extra_terms);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_subroutine);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_get_field_by_name);
    REGISTER_TEST_CASE(branch_tests::test_duplicate_destination_has_different_type);
    REGISTER_TEST_CASE(branch_tests::find_name_in_outer_branch);
    REGISTER_TEST_CASE(branch_tests::test_move);
    REGISTER_TEST_CASE(branch_tests::test_locals_stack);
}

}
} // namespace circa
