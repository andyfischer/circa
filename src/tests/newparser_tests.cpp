// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "circa.h"

namespace circa {
namespace newparser_tests {

void test_comment()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "-- this is a comment");

    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->state->field(0)->asString(), " this is a comment");
    test_assert(branch.numTerms() == 1);

    newparser::compile(branch, newparser::statement, "--");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->function == COMMENT_FUNC);
    test_equals(branch[1]->state->field(0)->asString(), "");
}

void test_blank_line()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "");
    test_assert(branch.numTerms() == 1);
    test_assert(branch[0]->function == COMMENT_FUNC);
    test_equals(branch[0]->state->field(0)->asString(), "");
}

void test_literal_integer()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
}

void test_literal_float()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "1.0");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
}

void test_literal_string()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "\"hello\"");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asString() == "hello");
}

void test_name_binding()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "a = 1");
    test_assert(branch.numTerms() == 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch[0] == branch["a"]);
    test_assert(branch[0]->name == "a");
}

void test_function_call()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "add(1.0,2.0)");
    test_assert(branch.numTerms() == 3);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asFloat() == 1.0);
    test_assert(is_value(branch[1]));
    test_assert(branch[1]->asFloat() == 2.0);

    test_assert(branch[2]->function == ADD_FUNC);
    test_assert(branch[2]->input(0) == branch[0]);
    test_assert(branch[2]->input(1) == branch[1]);
}

void test_identifier()
{
    Branch branch;
    newparser::compile(branch, newparser::statement, "a = 1.0");
    test_assert(branch.numTerms() == 1);

    Term* a = newparser::compile(branch, newparser::statement, "a");

    test_assert(branch.numTerms() == 1);
    test_assert(a == branch[0]);

    newparser::compile(branch, newparser::statement, "add(a,a)");
    test_assert(branch.numTerms() == 2);
    test_assert(branch[1]->input(0) == a);
    test_assert(branch[1]->input(1) == a);
}

void register_tests()
{
    REGISTER_TEST_CASE(newparser_tests::test_comment);
    REGISTER_TEST_CASE(newparser_tests::test_blank_line);
    REGISTER_TEST_CASE(newparser_tests::test_literal_integer);
    REGISTER_TEST_CASE(newparser_tests::test_literal_float);
    REGISTER_TEST_CASE(newparser_tests::test_literal_string);
    REGISTER_TEST_CASE(newparser_tests::test_name_binding);
    REGISTER_TEST_CASE(newparser_tests::test_function_call);
    REGISTER_TEST_CASE(newparser_tests::test_identifier);
}

} // namespace newparser_tests
} // namespace circa
