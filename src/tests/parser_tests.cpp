
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
    test_assert(literal->toString() == "\"quoted string\"");

    delete expr;
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

    delete functionCall;
}

void name_binding_expression()
{
    token_stream::TokenStream tokens("name = hi(2,u)");
    ast::ExpressionStatement *statement = parser::expressionStatement(tokens);

    test_assert(statement->nameBinding == "name");

    ast::FunctionCall *functionCall =
        dynamic_cast<ast::FunctionCall*>(statement->expression);

    test_assert(functionCall->functionName == "hi");
    test_assert(functionCall->toString() == "hi(2,u)");
    test_assert(statement->toString() == "name = hi(2,u)");

    delete statement;
}

void test_to_string()
{
    // Build an AST programatically so that the toString function
    // can't cheat.
    ast::FunctionCall *functionCall = new ast::FunctionCall("something");
    functionCall->addArgument(new ast::LiteralString("input0"), " ", "");
    functionCall->addArgument(new ast::LiteralInteger("4"), "", " ");
    test_assert(functionCall->toString() == "something( \"input0\",4 )");
    delete functionCall;
}

void create_literals()
{
    Branch branch;

    ast::LiteralInteger *lint = new ast::LiteralInteger("13");
    Term *int_t = lint->createTerm(&branch);
    test_assert(int_t->type == INT_TYPE);
    test_assert(as_int(int_t) == 13);
    delete lint;

    ast::LiteralFloat *lfloat = new ast::LiteralFloat("1.432");
    Term *float_t = lfloat->createTerm(&branch);
    test_assert(float_t->type == FLOAT_TYPE);
    test_assert(as_float(float_t) > 1.431 && as_float(float_t) < 1.433);
    delete lfloat;

    ast::LiteralString *lstr = new ast::LiteralString("hello");
    Term *str_t = lstr->createTerm(&branch);
    test_assert(str_t->type == STRING_TYPE);
    test_assert(as_string(str_t) == "hello");
    delete lstr;
}

void create_function_call()
{
    Branch branch;

    ast::FunctionCall *functionCall = new ast::FunctionCall("add");
    functionCall->addArgument(new ast::LiteralFloat("2"), "", "");
    functionCall->addArgument(new ast::LiteralFloat("3"), "", "");

    Term *term = functionCall->createTerm(&branch);
    test_assert(as_function(term->function)->name == "add");

    // make sure this term is not evaluated yet
    test_assert(term->needsUpdate);

    execute(term);

    test_assert(!term->needsUpdate);
    test_assert(as_float(term) == 5);
}

void test_quick_eval_statement()
{
    Branch branch;

    Term* result = parser::quick_eval_statement(&branch, "something = add(5.0,3.0)");

    test_assert(result != NULL);
    /*test_assert(result->findName() == "something");
    test_assert(as_function(result->function)->name == "add");
    test_assert(result->needsUpdate);
    execute(result);
    test_assert(as_float(result) == 8);*/
}

void function_decl_ast()
{
    // Create a FunctionDecl from scratch
    ast::FunctionDecl decl;
    decl.functionName = "add";
    decl.outputType = "float";
    decl.addArgument("float","arg1");
    decl.addArgument("float","arg2");

    decl.statements = new ast::StatementList();

    token_stream::TokenStream tokens("x = add(arg1,arg2)");
    decl.statements->push(parser::statement(tokens));

    test_assert(decl.toString() == "function add(float arg1, float arg2)\n{\nx = add(arg1,arg2)\n}");
}

void rebind_operator()
{
    token_stream::TokenStream tokens("@cheese");
    ast::Expression* expr = parser::atom(tokens);

    ast::Identifier* id = dynamic_cast<ast::Identifier*>(expr);
    test_assert(id != NULL);
    test_assert(id->text == "cheese");
    test_assert(id->hasRebindOperator);

    delete expr;

    tokens.reset("gouda");
    expr = parser::atom(tokens);
    id = dynamic_cast<ast::Identifier*>(expr);
    test_assert(id != NULL);
    test_assert(id->text == "gouda");
    test_assert(!id->hasRebindOperator);

    delete expr;

    tokens.reset("add(@a, 5)");
    ast::Statement* statement = parser::statement(tokens);

}

void ast_walk()
{
    token_stream::TokenStream tokens("concat(to-string(add(1,2.0)), \"cheese\")");
    ast::Expression* expr = parser::infixExpression(tokens);

}

} // namespace parser_tests

void register_parser_tests()
{
    REGISTER_TEST_CASE(parser_tests::literal_float);
    REGISTER_TEST_CASE(parser_tests::literal_string);
    REGISTER_TEST_CASE(parser_tests::function_call);
    REGISTER_TEST_CASE(parser_tests::name_binding_expression);
    REGISTER_TEST_CASE(parser_tests::test_to_string);
    REGISTER_TEST_CASE(parser_tests::create_literals);
    REGISTER_TEST_CASE(parser_tests::create_function_call);
    REGISTER_TEST_CASE(parser_tests::test_quick_eval_statement);
    REGISTER_TEST_CASE(parser_tests::function_decl_ast);
    REGISTER_TEST_CASE(parser_tests::rebind_operator);
    REGISTER_TEST_CASE(parser_tests::ast_walk);
}

} // namespace circa
