// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"

#include <circa.h>
#include "importing_macros.h"

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = create_float(branch, 2);
    Term* three = create_float(branch, 3);
    Term* negative_one = create_float(branch, -1);

    test_assert(as_float(apply_and_eval(branch, ADD_FUNC, RefList(two,three))) == 5);
    test_assert(as_float(apply_and_eval(branch, ADD_FUNC, RefList(two,negative_one))) == 1);

    apply_and_eval(branch, MULT_FUNC, RefList(two,three));
    test_assert(as_float(apply_and_eval(branch, MULT_FUNC, RefList(two,three))) == 6);
    test_assert(as_float(apply_and_eval(branch, MULT_FUNC, RefList(negative_one,three))) == -3);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE).formatSource != NULL);

    Term* four = create_int(branch, 4);
    Term* another_four = create_int(branch, 4);
    Term* five = create_int(branch, 5);

    test_assert(equals(four, another_four));
    test_assert(!equals(four, five));

    test_equals(four->toString(), "4");
}

void test_float()
{
    Branch branch;

    Type* floatType = &as_type(FLOAT_TYPE);

    test_assert(floatType->equals != NULL);
    test_assert(floatType->formatSource != NULL);

    Term* point_one = create_float(branch, .1f);
    Term* point_one_again = create_float(branch, .1f);
    Term* point_two = create_float(branch, 0.2f);

    test_assert(equals(point_one, point_one_again));
    test_assert(equals(point_two, point_two));

    test_equals(point_one->toString(), "0.1");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(branch.eval("cond(true, 'a', 'b')")) == "a");
    test_assert(as_string(branch.eval("cond(false, 'a', 'b')")) == "b");
}

void test_builtin_equals()
{
    Branch branch;
    EvalContext context;
    branch.compile("equals(5.0, add)");
    evaluate_branch(&context, branch);
    test_assert(context.errorOccurred);
}

void test_list()
{
    Branch branch;
    Term* l = branch.eval("l = list(1,2,\"foo\")");

    test_assert(l->getIndex(0)->asInt() == 1);
    test_assert(l->getIndex(1)->asInt() == 2);
    test_assert(l->getIndex(2)->asString() == "foo");
}

void test_range()
{
    Branch branch;
    Term* t = branch.eval("range(0,5)");

    test_assert(t->numElements() == 5);
    for (int i=0; i < 5; i++)
        test_assert(t->getIndex(i)->asInt() == i);

    // there was once a bug where the result list would grow on every call
    evaluate_term(t);
    evaluate_term(t);
    evaluate_term(t);
    evaluate_term(t);
    test_assert(t->numElements() == 5);
}

void test_map()
{
    Branch branch;
    branch.eval("input_list = [1 2 3 4 5]");
    Term* map_sqr = branch.eval("map(sqr, input_list)");

    test_assert(map_sqr);

}

void test_vectorized_funcs()
{
    Branch branch;
    Term* t = branch.eval("[1 2 3] + [4 5 6]");
    test_assert(t);

    test_assert(t->numElements() == 3);
    test_assert(t->getIndex(0)->asInt() == 5);
    test_assert(t->getIndex(1)->asInt() == 7);
    test_assert(t->getIndex(2)->asInt() == 9);

    // Test mult_s (multiply a vector to a scalar)
    t = branch.eval("[10 20 30] * 1.1");

    //dump_branch(branch);

    test_assert(t->numElements() == 3);
    test_equals(t->getIndex(0)->asFloat(), 11);
    test_equals(t->getIndex(1)->asFloat(), 22);
    test_equals(t->getIndex(2)->asFloat(), 33);

    // Test error handling
    t = branch.eval("[1 1 1] + [1 1]");
    EvalContext context;
    evaluate_term(&context, t);
    test_assert(context.errorOccurred);
}

void test_vectorized_funcs_with_points()
{
    // This test is similar, but we define the type Point and see if
    // vectorized functions work against that.
    Branch branch;
    
    Term* point_t = branch.eval("type Point {number x, number y}");

    Term* a = branch.eval("a = [1 0] -> Point");

    test_assert(a->type == point_t);

    Term* b = branch.eval("b = a + [0 2]");

    // Make sure 'b' was resolved to the correct overload (previous bug)
    test_assert(inputs_statically_fit_function(
                overloaded_function::find_overload(ADD_FUNC, "add_v"), b->inputs));
    test_equals(overloaded_function::statically_specialize_function(ADD_FUNC, b->inputs)->name,
            "add_v");
            
    test_equals(b->function->name, "add_v");

    test_equals(b->getIndex(0)->toFloat(), 1);
    test_equals(b->getIndex(1)->toFloat(), 2);
}

void test_cond_with_int_and_float()
{
    Branch branch;

    // This code once caused a bug
    Term* a = branch.eval("cond(true, 1, 1.0)");
    test_assert(a->type != ANY_TYPE);
    Term* b = branch.eval("cond(true, 1.0, 1)");
    test_assert(b->type != ANY_TYPE);
}

void test_get_index()
{
    Branch branch;

    branch.eval("l = [1 2 3]");
    Term* get = branch.eval("get_index(l, 0)");

    test_assert(get);
    test_assert(get->value_type == type_contents(INT_TYPE));
    test_assert(get->asInt() == 1);

    branch.eval("l = []");
    get = branch.eval("get_index(l, 5)");
    EvalContext context;
    evaluate_term(&context, get);
    test_assert(context.errorOccurred);
}

void test_set_index()
{
    Branch branch;

    branch.eval("l = [1 2 3]");
    Term* l2 = branch.eval("set_index(@l, 1, 5)");

    test_assert(l2);
    test_assert(l2->getIndex(0)->asInt() == 1);
    test_assert(l2->getIndex(1)->asInt() == 5);
    test_assert(l2->getIndex(2)->asInt() == 3);

    Term* l3 = branch.eval("l[2] = 9");
    test_assert(l3);
    test_assert(l3->getIndex(0)->asInt() == 1);
    test_assert(l3->getIndex(1)->asInt() == 5);
    test_assert(l3->getIndex(2)->asInt() == 9);
}

void test_do_once()
{
    Branch branch;
    Term* x = branch.eval("x = 1");
    Term* t = branch.compile("do_once()");
    t->asBranch().compile("unsafe_assign(x,2)");

    test_assert(as_int(x) == 1);

    // the assign() inside do_once should modify x
    evaluate_branch(branch);
    test_assert(as_int(x) == 2);

    // but if we call it again, it shouldn't do that any more
    set_int(x, 3);
    evaluate_branch(branch);
    test_assert(as_int(x) == 3);

    branch.clear();
}

void test_changed()
{
    Branch branch;
    Term* x = branch.compile("x = 5");
    Term* changed = branch.compile("changed(x)");

    evaluate_branch(branch);
    test_assert(changed->asBool() == true);

    evaluate_branch(branch);
    test_assert(changed->asBool() == false);
    evaluate_branch(branch);
    test_assert(changed->asBool() == false);

    set_int(x, 6);
    evaluate_branch(branch);
    test_assert(changed->asBool() == true);
    evaluate_branch(branch);
    test_assert(changed->asBool() == false);
}

void test_message_passing()
{
    Branch branch;
    Term* i = branch.compile("i = inbox()");
    Term* send = branch.compile("send(i, 1)");

    // Before running, i should be empty
    test_assert(i->numElements() == 0);
    test_assert(get_hidden_state_for_call(i)->numElements() == 0);

    // First run, i is still empty, but the hidden state has 1
    evaluate_branch(branch);
    test_assert(i->numElements() == 0);
    test_assert(get_hidden_state_for_call(i)->numElements() == 1);

    // Second run, i now returns 1
    evaluate_branch(branch);
    test_assert(i->numElements() == 1);
    test_assert(i->getIndex(0)->asInt() == 1);
    test_assert(get_hidden_state_for_call(i)->numElements() == 1);

    // Delete the send() call
    branch.remove(send);

    // Third run, i still returns 1 (from previous call), hidden state is empty
    evaluate_branch(branch);
    test_assert(i->numElements() == 1);
    test_assert(i->getIndex(0)->asInt() == 1);
    test_assert(get_hidden_state_for_call(i)->numElements() == 0);

    // Fourth run, i is empty again
    evaluate_branch(branch);
    test_assert(i->numElements() == 0);
    test_assert(get_hidden_state_for_call(i)->numElements() == 0);
}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_math);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
    REGISTER_TEST_CASE(builtin_function_tests::test_builtin_equals);
    REGISTER_TEST_CASE(builtin_function_tests::test_list);
    REGISTER_TEST_CASE(builtin_function_tests::test_range);
    REGISTER_TEST_CASE(builtin_function_tests::test_map);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs_with_points);
    REGISTER_TEST_CASE(builtin_function_tests::test_cond_with_int_and_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_get_index);
    REGISTER_TEST_CASE(builtin_function_tests::test_set_index);
    REGISTER_TEST_CASE(builtin_function_tests::test_do_once);
    REGISTER_TEST_CASE(builtin_function_tests::test_changed);
    REGISTER_TEST_CASE(builtin_function_tests::test_message_passing);
}

} // namespace builtin_function_tests

} // namespace circa
