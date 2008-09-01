
#include "common_headers.h"

#include "circa.h"
#include <tests/common.h>

namespace circa {
namespace builtin_function_tests {

void test_math()
{
    Branch branch;

    Term* two = constant_float(&branch, 2);
    Term* three = constant_float(&branch, 3);
    Term* negative_one = constant_float(&branch, -1);
    Term* add_f = get_global("add");
    Term* mult_f = get_global("mult");

    test_assert(as_float(exec_function(&branch, add_f, TermList(two,three))) == 5);
    test_assert(as_float(exec_function(&branch, add_f, TermList(two,negative_one))) == 1);

    test_assert(as_float(exec_function(&branch, mult_f, TermList(two,three))) == 6);
    test_assert(as_float(exec_function(&branch, mult_f, TermList(negative_one,three))) == -3);

    test_assert(as_float(parser::quick_exec_statement(&branch, "mult(5.0,3.0)")) == 15);
}

void test_int()
{
    Branch branch;

    test_assert(as_type(INT_TYPE)->equals != NULL);

    Term* four = constant_int(&branch, 4);
    Term* another_four = constant_int(&branch, 4);
    Term* five = constant_int(&branch, 5);

    test_assert(four->equals(another_four));
    test_assert(!four->equals(five));
}

void test_float()
{
    Branch branch;

    test_assert(as_type(FLOAT_TYPE)->equals != NULL);

    Term* point_one = constant_float(&branch, .1);
    Term* point_one_again = constant_float(&branch, .1);
    Term* point_two = constant_float(&branch, 0.2);
    test_assert(point_one->equals(point_one));
    test_assert(point_two->equals(point_two));
}

void test_string()
{
    Branch branch;

    test_assert(as_string(parser::quick_exec_statement(&branch, "concat(\"hello \", \"world\")"))
            == "hello world");
}

void test_bool()
{
    Branch branch;

    test_assert(as_string(parser::quick_exec_statement(&branch, "if-expr(true, 'a, 'b)")) == "a");
    test_assert(as_string(parser::quick_exec_statement(&branch, "if-expr(false, 'a, 'b)")) == "b");
}

} // namespace builtin_function_tests

void register_builtin_function_tests()
{
    REGISTER_TEST_CASE(builtin_function_tests::test_int);
    REGISTER_TEST_CASE(builtin_function_tests::test_float);
    REGISTER_TEST_CASE(builtin_function_tests::test_math);
    REGISTER_TEST_CASE(builtin_function_tests::test_string);
    REGISTER_TEST_CASE(builtin_function_tests::test_bool);
}

} // namespace circa
