// Copyright 2008 Andrew Fischer

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
    test_assert(branch.numTerms() >= 1);
    test_assert(is_value(branch[0]));
    test_assert(branch[0]->asInt() == 1);
    test_assert(branch.numTerms() == 1);
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
void register_tests()
{
    REGISTER_TEST_CASE(newparser_tests::test_comment);
    REGISTER_TEST_CASE(newparser_tests::test_blank_line);
    REGISTER_TEST_CASE(newparser_tests::test_literal_integer);
    REGISTER_TEST_CASE(newparser_tests::test_literal_float);
    REGISTER_TEST_CASE(newparser_tests::test_literal_string);
}

} // namespace newparser_tests
} // namespace circa
