// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "testing.h"
#include "branch.h"
#include "builtins.h"
#include "function.h"
#include "introspection.h"
#include "parser.h"
#include "runtime.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = float_var(branch, 2);
    Term* three = float_var(branch, 3);
    Term* negative_one = float_var(branch, -1);

    test_assert(as_float(eval_function(branch, ADD_FUNC, ReferenceList(two,three))) == 5);
    test_assert(as_float(eval_function(branch, ADD_FUNC, ReferenceList(two,negative_one))) == 1);

    eval_function(branch, MULT_FUNC, ReferenceList(two,three));
    test_assert(as_float(eval_function(branch, MULT_FUNC, ReferenceList(two,three))) == 6);
    test_assert(as_float(eval_function(branch, MULT_FUNC, ReferenceList(negative_one,three))) == -3);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE).equals != NULL);
    test_assert(as_type(INT_TYPE).toString != NULL);

    Term* four = int_var(branch, 4);
    Term* another_four = int_var(branch, 4);
    Term* five = int_var(branch, 5);

    test_assert(four->equals(another_four));
    test_assert(!four->equals(five));

    test_assert(four->toString() == "4");
}

void test_float()
{
    Branch branch;

    test_assert(as_type(FLOAT_TYPE).equals != NULL);
    test_assert(as_type(FLOAT_TYPE).toString != NULL);

    Term* point_one = float_var(branch, .1);
    Term* point_one_again = float_var(branch, .1);
    Term* point_two = float_var(branch, 0.2);

    test_assert(point_one->equals(point_one_again));
    test_assert(point_two->equals(point_two));

    test_assert(point_one->toString() == "0.1");
}

void test_string()
{
    Branch branch;

    test_assert(as_string(eval_statement(branch, "concat(\"hello \", \"world\")"))
            == "hello world");
}

void test_concat()
{
    Branch branch;

    //TODO
    //test_assert(eval_as<std::string>("concat('a ', 'b', ' c')") == "a b c");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(eval_statement(branch, "if-expr(true, 'a', 'b')")) == "a");
    test_assert(as_string(eval_statement(branch, "if-expr(false, 'a', 'b')")) == "b");
}

void test_reference()
{
    Branch branch;

    Term* myref = create_var(&branch, REFERENCE_TYPE);
    Term* a = create_var(&branch, INT_TYPE);
    Term* b = create_var(&branch, INT_TYPE);

    as_ref(myref) = a;

    test_equals(list_all_pointers(myref),
        ReferenceList(myref->function, REFERENCE_TYPE, a));

    ReferenceMap myMap;
    myMap[a] = b;

    remap_pointers(myref, myMap);

    test_assert(as_ref(myref) == b);

    test_equals(list_all_pointers(myref),
        ReferenceList(myref->function, REFERENCE_TYPE, b));

    myMap[b] = NULL;
    remap_pointers(myref, myMap);

    test_assert(as_ref(myref) == NULL);

    test_equals(list_all_pointers(myref),
        ReferenceList(myref->function, REFERENCE_TYPE));
}

} // namespace builtin_function_tests

void register_builtin_function_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_math);
    REGISTER_TEST_CASE(builtin_function_tests::test_string);
    REGISTER_TEST_CASE(builtin_function_tests::test_concat);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
    REGISTER_TEST_CASE(builtin_function_tests::test_reference);
}

} // namespace circa
