// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include <circa.h>

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = float_value(&branch, 2);
    Term* three = float_value(&branch, 3);
    Term* negative_one = float_value(&branch, -1);

    test_assert(as_float(apply_and_eval(&branch, ADD_FUNC, RefList(two,three))) == 5);
    test_assert(as_float(apply_and_eval(&branch, ADD_FUNC, RefList(two,negative_one))) == 1);

    apply_and_eval(&branch, MULT_FUNC, RefList(two,three));
    test_assert(as_float(apply_and_eval(&branch, MULT_FUNC, RefList(two,three))) == 6);
    test_assert(as_float(apply_and_eval(&branch, MULT_FUNC, RefList(negative_one,three))) == -3);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE).equals != NULL);
    test_assert(as_type(INT_TYPE).toString != NULL);

    Term* four = int_value(&branch, 4);
    Term* another_four = int_value(&branch, 4);
    Term* five = int_value(&branch, 5);

    test_assert(equals(four, another_four));
    test_assert(!equals(four, five));

    test_assert(four->toString() == "4");
}

void test_float()
{
    Branch branch;

    test_assert(as_type(FLOAT_TYPE).equals != NULL);
    test_assert(as_type(FLOAT_TYPE).toString != NULL);

    Term* point_one = float_value(&branch, .1);
    Term* point_one_again = float_value(&branch, .1);
    Term* point_two = float_value(&branch, 0.2);

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

    test_assert(eval_as<std::string>("concat('a ', 'b', ' c')") == "a b c");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(branch.eval("if_expr(true, 'a', 'b')")) == "a");
    test_assert(as_string(branch.eval("if_expr(false, 'a', 'b')")) == "b");
}

void test_builtin_equals()
{
    test_assert(eval_as<bool>("equals(1,1)"));
    test_assert(!eval_as<bool>("equals(1,2)"));
    test_assert(eval_as<bool>("equals('hello','hello')"));
    test_assert(!eval_as<bool>("equals('hello','goodbye')"));

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
    Term* t = branch.eval("map(sqr, [1 2 3 4 5])");

    test_assert(t);

    Branch& result = as_branch(t);

    test_assert(result.length() == 5);
    test_equals(result[0]->asFloat(), 1);
    test_equals(result[1]->asFloat(), 4);
    test_equals(result[2]->asFloat(), 9);
    test_equals(result[3]->asFloat(), 16);
    test_equals(result[4]->asFloat(), 25);

    // Test with subroutines
    branch.clear();

    branch.eval("def myfunc(float x):float\nreturn x + 5\nend");
    t = branch.eval("map(myfunc, [1 2 3 4 5])");
    test_assert(t);

    Branch& result2 = as_branch(t);
    test_assert(result2.length() == 5);
    test_equals(result2[0]->asFloat(), 6);
    test_equals(result2[1]->asFloat(), 7);
    test_equals(result2[2]->asFloat(), 8);
    test_equals(result2[3]->asFloat(), 9);
    test_equals(result2[4]->asFloat(), 10);
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
}

} // namespace builtin_function_tests

} // namespace circa
