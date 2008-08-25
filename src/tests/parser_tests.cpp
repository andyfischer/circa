
#include "common_headers.h"

#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace parser_tests {

void atoms()
{
    token_stream::TokenStream tokens("1.0");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralFloat *literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "1.0");
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
    REGISTER_TEST_CASE(parser_tests::atoms);
    REGISTER_TEST_CASE(parser_tests::function_call);
    REGISTER_TEST_CASE(parser_tests::name_binding_expression);
}

} // namespace circa
