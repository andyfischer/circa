
#include "common_headers.h"

#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace parser_tests {

void literal_float()
{
    token_stream::TokenStream tokens("1.0");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralFloat *literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "1.0");
    test_assert(literal->toString() == "1.0");
}

void literal_string()
{
    token_stream::TokenStream tokens("\"quoted string\"");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralString *literal = dynamic_cast<ast::LiteralString*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "quoted string");
    test_assert(literal->toString() == "quoted string");
}

void function_call()
{
    token_stream::TokenStream tokens("add(1,2)");
    ast::FunctionCall *functionCall = parser::functionCall(tokens);
    test_assert(functionCall != NULL);
    test_assert(functionCall->functionName == "add");

    ast::LiteralInteger *arg1 =
        dynamic_cast<ast::LiteralInteger*>(functionCall->arguments[0]->expression);
    test_assert(arg1 != NULL);
    test_assert(arg1->text == "1");
    ast::LiteralInteger *arg2 =
        dynamic_cast<ast::LiteralInteger*>(functionCall->arguments[1]->expression);
    test_assert(arg2 != NULL);
    test_assert(arg2->text == "2");
    test_assert(functionCall->toString() == "add(1,2)");
}

void name_binding_expression()
{
    token_stream::TokenStream tokens("name = hi(2,u)");
    ast::Statement *statement = parser::statement(tokens);

    test_assert(statement->nameBinding == "name");

    ast::FunctionCall *functionCall =
        dynamic_cast<ast::FunctionCall*>(statement->expression);

    test_assert(functionCall->functionName == "hi");
    test_assert(functionCall->toString() == "hi(2,u)");
    test_assert(statement->toString() == "name = hi(2,u)");
}

} // namespace parser_tests

void register_parser_tests()
{
    REGISTER_TEST_CASE(parser_tests::literal_float);
    REGISTER_TEST_CASE(parser_tests::literal_string);
    REGISTER_TEST_CASE(parser_tests::function_call);
    REGISTER_TEST_CASE(parser_tests::name_binding_expression);
}

} // namespace circa
