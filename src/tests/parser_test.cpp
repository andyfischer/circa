
#include "common_headers.h"

#include "tests/common.h"
#include "circa.h"

namespace circa {
namespace parser_test {

void test_atoms()
{
    token_stream::TokenStream tokens("1.0");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralFloat *literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "1.0");
}

void test_function_call()
{
    token_stream::TokenStream tokens("add(1,2)");
    ast::FunctionCall *functionCall =
        dynamic_cast<ast::FunctionCall*>(parser::functionCall(tokens));
    test_assert(functionCall != NULL);
    test_assert(functionCall->functionName == "add");

    ast::LiteralInteger
    

}

void all_tests()
{
    test_atoms();
}

} // namespace parser_test
} // namespace circa
