// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "cpp_importing.h"
#include "testing.h"
#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = float_value(branch, 2);
    Term* three = float_value(branch, 3);
    Term* negative_one = float_value(branch, -1);

    test_assert(as_float(eval_function(branch, ADD_FUNC, RefList(two,three))) == 5);
    test_assert(as_float(eval_function(branch, ADD_FUNC, RefList(two,negative_one))) == 1);

    eval_function(branch, MULT_FUNC, RefList(two,three));
    test_assert(as_float(eval_function(branch, MULT_FUNC, RefList(two,three))) == 6);
    test_assert(as_float(eval_function(branch, MULT_FUNC, RefList(negative_one,three))) == -3);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE).equals != NULL);
    test_assert(as_type(INT_TYPE).toString != NULL);

    Term* four = int_value(branch, 4);
    Term* another_four = int_value(branch, 4);
    Term* five = int_value(branch, 5);

    test_assert(values_equal(four, another_four));
    test_assert(!values_equal(four, five));

    test_assert(four->toString() == "4");
}

void test_float()
{
    Branch branch;

    test_assert(as_type(FLOAT_TYPE).equals != NULL);
    test_assert(as_type(FLOAT_TYPE).toString != NULL);

    Term* point_one = float_value(branch, .1);
    Term* point_one_again = float_value(branch, .1);
    Term* point_two = float_value(branch, 0.2);

    test_assert(values_equal(point_one, point_one_again));
    test_assert(values_equal(point_two, point_two));

    test_assert(point_one->toString() == "0.1");
}

void test_string()
{
    Branch branch;

    test_equals(as_string(branch.eval("concat(\"hello \", \"world\")")),
            "hello world");
}

void test_concat()
{
    Branch branch;

    test_assert(eval_as<std::string>("concat('a ', 'b', ' c')") == "a b c");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(branch.eval("if-expr(true, 'a', 'b')")) == "a");
    test_assert(as_string(branch.eval("if-expr(false, 'a', 'b')")) == "b");
}

void test_reference()
{
    Branch branch;

    Term* myref = create_value(&branch, REF_TYPE);
    Term* a = create_value(&branch, INT_TYPE);
    Term* b = create_value(&branch, INT_TYPE);

    as_ref(myref) = a;

    RefList refs = reference_iterator_to_list(start_reference_iterator(myref));
    test_equals(refs, RefList(a));

    ReferenceMap myMap;
    myMap[a] = b;

    remap_pointers(myref, myMap);

    test_assert(as_ref(myref) == b);

    refs = reference_iterator_to_list(start_reference_iterator(myref));
    test_equals(refs, RefList(b));

    myMap[b] = NULL;
    remap_pointers(myref, myMap);

    test_assert(as_ref(myref) == NULL);

    refs = reference_iterator_to_list(start_reference_iterator(myref));
    test_equals(refs, RefList(NULL));
}

void test_builtin_equals()
{
    test_assert(eval_as<bool>("equals(1,1)"));
    test_assert(!eval_as<bool>("equals(1,2)"));
    test_assert(eval_as<bool>("equals('hello','hello')"));
    test_assert(!eval_as<bool>("equals('hello','goodbye')"));

    Branch branch;
    Term* term = branch.eval("equals(5.0, add)");
    test_assert(term->hasError());
}

void test_map()
{
    Branch branch;

    branch.eval("ages = map(string, int)");
    branch.eval("ages('Henry') := 11");
    branch.eval("ages('Absalom') := 205");

    test_assert(branch.eval("ages('Henry')")->asInt() == 11);
    test_assert(branch.eval("ages('Absalom')")->asInt() == 205);
}

void test_alias()
{
    Branch branch;

    Term *five = branch.eval("five = 5");
    Term *a = branch.eval("a = alias(five)");

    test_assert(a);
    test_assert(as_int(a) == 5);

    as_int(five) = 55;

    test_assert(as_int(a) == 55);
}

void test_for()
{
    Branch branch;
    //Term* l = branch.eval("l = list(1,2,3)");
    //Term* f = apply_function(&branch, get_global("for"), RefList(l));
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

void register_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_math);
    REGISTER_TEST_CASE(builtin_function_tests::test_string);
    REGISTER_TEST_CASE(builtin_function_tests::test_concat);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
    REGISTER_TEST_CASE(builtin_function_tests::test_reference);
    REGISTER_TEST_CASE(builtin_function_tests::test_builtin_equals);
    REGISTER_TEST_CASE(builtin_function_tests::test_map);
    REGISTER_TEST_CASE(builtin_function_tests::test_alias);
    REGISTER_TEST_CASE(builtin_function_tests::test_list);
}

} // namespace builtin_function_tests

} // namespace circa
