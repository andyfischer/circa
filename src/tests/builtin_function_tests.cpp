// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "cpp_interface.h"
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

    Term* two = float_value(branch, 2);
    Term* three = float_value(branch, 3);
    Term* negative_one = float_value(branch, -1);

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

    Term* four = int_value(branch, 4);
    Term* another_four = int_value(branch, 4);
    Term* five = int_value(branch, 5);

    test_assert(four->equals(another_four));
    test_assert(!four->equals(five));

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

    test_assert(eval_as<std::string>("concat('a ', 'b', ' c')") == "a b c");
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

    Term* myref = create_value(&branch, REFERENCE_TYPE);
    Term* a = create_value(&branch, INT_TYPE);
    Term* b = create_value(&branch, INT_TYPE);

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

void test_parse_function_header()
{
    ast::FunctionHeader header = eval_as<ast::FunctionHeader>(
        "parse-function-header('function fname(int arg1, string arg2) -> float'.tokenize)");

    test_assert(header.functionName == "fname");
    test_assert(header.arguments[0].name == "arg1");
    test_assert(header.arguments[0].type == "int");
    test_assert(header.outputType == "float");
}

void test_builtin_equals()
{
    test_assert(eval_as<bool>("equals(1,1)"));
    test_assert(!eval_as<bool>("equals(1,2)"));
    test_assert(eval_as<bool>("equals('hello','hello')"));
    test_assert(!eval_as<bool>("equals('hello','goodbye')"));

    Branch branch;
    Term* term = eval_statement(branch, "equals(5.0, add)");
    test_assert(term->hasError());
}

void test_map()
{
    Branch branch;

    eval_statement(branch, "ages = map(string, int)");
    eval_statement(branch, "ages('Henry') := 11");
    eval_statement(branch, "ages('Absalom') := 205");

    test_assert(eval_statement(branch, "ages('Henry')")->asInt() == 11);
    test_assert(eval_statement(branch, "ages('Absalom')")->asInt() == 205);
}

void test_if_statement()
{
    Branch branch;

    Term* condition = eval_statement(branch, "cond = true");
    Term* if_statement = eval_statement(branch, "if_s = if-statement(cond)");

    Branch &positiveBranch = as_branch(if_statement->state->field(0));
    Branch &negativeBranch = as_branch(if_statement->state->field(1));

    Term* posTerm = apply_statement(positiveBranch, "x = 1.0 + 1.0");
    Term* negTerm = apply_statement(negativeBranch, "x = 1.0 + 1.0");

    dealloc_value(posTerm);
    dealloc_value(negTerm);

    test_assert(posTerm->value == NULL);
    test_assert(negTerm->value == NULL);

    evaluate_branch(branch);

    test_assert(as_float(posTerm) == 2.0);
    test_assert(negTerm->value == NULL);

    dealloc_value(posTerm);

    as_bool(condition) = false;

    evaluate_branch(branch);

    test_assert(posTerm->value == NULL);
    test_assert(as_float(negTerm) == 2.0);
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
    REGISTER_TEST_CASE(builtin_function_tests::test_parse_function_header);
    REGISTER_TEST_CASE(builtin_function_tests::test_builtin_equals);
    REGISTER_TEST_CASE(builtin_function_tests::test_map);
    REGISTER_TEST_CASE(builtin_function_tests::test_if_statement);
}

} // namespace circa
