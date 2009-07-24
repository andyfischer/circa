// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = float_value(branch, 2);
    Term* three = float_value(branch, 3);
    Term* negative_one = float_value(branch, -1);

    test_assert(as_float(apply_and_eval(branch, ADD_FUNC, RefList(two,three))) == 5);
    test_assert(as_float(apply_and_eval(branch, ADD_FUNC, RefList(two,negative_one))) == 1);

    apply_and_eval(branch, MULT_FUNC, RefList(two,three));
    test_assert(as_float(apply_and_eval(branch, MULT_FUNC, RefList(two,three))) == 6);
    test_assert(as_float(apply_and_eval(branch, MULT_FUNC, RefList(negative_one,three))) == -3);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE).equals != NULL);
    test_assert(as_type(INT_TYPE).toString != NULL);

    Term* four = int_value(branch, 4);
    Term* another_four = int_value(branch, 4);
    Term* five = int_value(branch, 5);

    test_assert(equals(four, another_four));
    test_assert(!equals(four, five));

    test_assert(four->toString() == "4");
}

void test_float()
{
    Branch branch;

    test_assert(as_type(FLOAT_TYPE).equals != NULL);
    test_assert(as_type(FLOAT_TYPE).toString != NULL);

    Term* point_one = float_value(branch, .1f);
    Term* point_one_again = float_value(branch, .1f);
    Term* point_two = float_value(branch, 0.2f);

    test_assert(equals(point_one, point_one_again));
    test_assert(equals(point_two, point_two));

    test_equals(point_one->toString(), "0.1");
}

void test_string()
{
    Branch branch;

    test_equals(as_string(branch.eval("concat(\"hello \", \"world\")")), "hello world");
}

void test_concat()
{
    Branch branch;

    test_assert(eval<std::string>("concat('a ', 'b', ' c')") == "a b c");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(branch.eval("if_expr(true, 'a', 'b')")) == "a");
    test_assert(as_string(branch.eval("if_expr(false, 'a', 'b')")) == "b");
}

void test_builtin_equals()
{
    test_assert(eval<bool>("equals(1,1)"));
    test_assert(!eval<bool>("equals(1,2)"));
    test_assert(eval<bool>("equals('hello','hello')"));
    test_assert(!eval<bool>("equals('hello','goodbye')"));

    Branch branch;
    Term* term = branch.eval("equals(5.0, add)");
    test_assert(term->hasError);
}

void test_list()
{
    Branch branch;
    Term* l = branch.eval("l = list(1,2,\"foo\")");

    test_assert(is_branch(l));
    test_assert(as_branch(l)[0]->asInt() == 1);
    test_assert(as_branch(l)[1]->asInt() == 2);
    test_assert(as_branch(l)[2]->asString() == "foo");
}

void test_range()
{
    Branch branch;
    Term* t = branch.eval("range(5)");

    test_assert(as_branch(t).length() == 5);
    for (int i=0; i < 5; i++)
        test_assert(as_int(as_branch(t)[i]) == i);

    // there was once a bug where the result list would grow on every call
    evaluate_term(t);
    evaluate_term(t);
    evaluate_term(t);
    evaluate_term(t);
    test_assert(as_branch(t).length() == 5);
}

void test_map()
{
    Branch branch;
    Term* input_list = branch.eval("input_list = [1 2 3 4 5]");
    Term* map_sqr = branch.eval("map(sqr, input_list)");

    test_assert(map_sqr);

    Branch& result = as_branch(map_sqr);

    test_assert(result.length() == 5);
    test_equals(result[0]->asFloat(), 1);
    test_equals(result[1]->asFloat(), 4);
    test_equals(result[2]->asFloat(), 9);
    test_equals(result[3]->asFloat(), 16);
    test_equals(result[4]->asFloat(), 25);

    // Test with subroutines
    branch.eval("def myfunc(float x):float\nreturn x + 5\nend");
    Term* map_myfunc = branch.eval("map(myfunc, input_list)");

    test_assert(map_myfunc);

    Branch& result2 = as_branch(map_myfunc);
    test_assert(result2.length() == 5);
    test_equals(result2[0]->asFloat(), 6);
    test_equals(result2[1]->asFloat(), 7);
    test_equals(result2[2]->asFloat(), 8);
    test_equals(result2[3]->asFloat(), 9);
    test_equals(result2[4]->asFloat(), 10);

    // Make sure that if the input list changes size, the size of the result
    // list is adjusted too.

    // Test shrinking the input list
    resize_list(as_branch(input_list), 3, INT_TYPE);
    evaluate_branch(branch);
    test_assert(as_branch(map_sqr).length() == 3);
    test_assert(as_branch(map_myfunc).length() == 3);

    // Test growing the input list
    resize_list(as_branch(input_list), 6, INT_TYPE);
    evaluate_branch(branch);
    test_assert(as_branch(map_sqr).length() == 6);
    test_assert(as_branch(map_myfunc).length() == 6);
}

void test_vectorized_funcs()
{
    Branch branch;
    Term* t = branch.eval("[1 2 3] + [4 5 6]");
    test_assert(t);
    test_assert(is_branch(t));

    Branch& result = as_branch(t);
    
    test_assert(result.length() == 3);
    test_equals(result[0]->function->name, "add_i");
    test_equals(result[1]->function->name, "add_i");
    test_equals(result[2]->function->name, "add_i");
    test_assert(result[0]->asInt() == 5);
    test_assert(result[1]->asInt() == 7);
    test_assert(result[2]->asInt() == 9);

    // Test mult_s (multiply a vector to a scalar)
    t = branch.eval("[10 20 30] * 1.1");
    Branch& mult_result = as_branch(t);
    test_assert(mult_result.length() == 3);
    test_equals(mult_result[0]->function->name, "mult_f");
    test_equals(mult_result[1]->function->name, "mult_f");
    test_equals(mult_result[2]->function->name, "mult_f");
    test_equals(mult_result[0]->asFloat(), 11);
    test_equals(mult_result[1]->asFloat(), 22);
    test_equals(mult_result[2]->asFloat(), 33);
}

void test_vectorized_funcs_with_points()
{
    // This test is similar, but we define the type Point and see if
    // vectorized functions work against that.
    Branch branch;
    
    Term* point_t = branch.eval("type Point {float x, float y}");

    Term* a = branch.eval("a = [1 0] : Point");

    test_assert(a->type == point_t);

    Term* b = branch.eval("a + [0 2]");

    test_equals(b->field(0)->toFloat(), 1);
    test_equals(b->field(1)->toFloat(), 2);
}

void test_if_expr_with_int_and_float()
{
    Branch branch;

    // This code once caused a bug
    Term* a = branch.eval("if_expr(true, 1, 1.0)");
    test_assert(a->type != ANY_TYPE);
    Term* b = branch.eval("if_expr(true, 1.0, 1)");
    test_assert(b->type != ANY_TYPE);
}

void test_get_index()
{
    Branch branch;

    branch.eval("l = [1 2 3]");
    Term* get = branch.eval("get_index(l, 0)");

    test_assert(get);
    test_assert(get->type == INT_TYPE);
    test_assert(get->asInt() == 1);

    branch.eval("l = []");
    get = branch.eval("get_index(l, 5)");
    test_assert(get->hasError);
}

void test_set_index()
{
    Branch branch;

    branch.eval("l = [1 2 3]");
    Term* l2 = branch.eval("set_index(@l, 1, 5)");

    test_assert(l2);
    test_assert(l2->field(0)->asInt() == 1);
    test_assert(l2->field(1)->asInt() == 5);
    test_assert(l2->field(2)->asInt() == 3);

    Term* l3 = branch.eval("l[2] = 9");
    test_assert(l3);
    test_assert(l3->field(0)->asInt() == 1);
    test_assert(l3->field(1)->asInt() == 5);
    test_assert(l3->field(2)->asInt() == 9);
}

void test_do_once()
{
    Branch branch;
    Term* x = branch.eval("x = 1");
    Term* t = branch.compile("do_once()");
    t->asBranch().compile("assign(x,2)");

    test_assert(as_int(x) == 1);

    // the assign() inside do_once should modify x
    evaluate_branch(branch);
    test_assert(as_int(x) == 2);

    // but if we call it again, it shouldn't do that any more
    as_int(x) = 3;
    evaluate_branch(branch);
    test_assert(as_int(x) == 3);

    branch.clear();

}

void register_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_math);
    REGISTER_TEST_CASE(builtin_function_tests::test_string);
    REGISTER_TEST_CASE(builtin_function_tests::test_concat);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
    REGISTER_TEST_CASE(builtin_function_tests::test_builtin_equals);
    REGISTER_TEST_CASE(builtin_function_tests::test_list);
    REGISTER_TEST_CASE(builtin_function_tests::test_range);
    REGISTER_TEST_CASE(builtin_function_tests::test_map);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs);
    REGISTER_TEST_CASE(builtin_function_tests::test_vectorized_funcs_with_points);
    REGISTER_TEST_CASE(builtin_function_tests::test_if_expr_with_int_and_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_get_index);
    REGISTER_TEST_CASE(builtin_function_tests::test_set_index);
    REGISTER_TEST_CASE(builtin_function_tests::test_do_once);
}

} // namespace builtin_function_tests

} // namespace circa
