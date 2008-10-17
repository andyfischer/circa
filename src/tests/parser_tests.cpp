// Copyright 2008 Andrew Fischer

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
    test_assert(literal->hasQuestionMark == false);

    delete expr;

    tokens.reset("5.0?");
    expr = parser::atom(tokens);
    literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "5.0");
    test_assert(literal->toString() == "5.0");
    test_assert(literal->hasQuestionMark == true);

    delete expr;
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
    Term *int_t = lint->createTerm(branch);
    test_assert(int_t->type == INT_TYPE);
    test_assert(as_int(int_t) == 13);
    delete lint;

    ast::LiteralFloat *lfloat = new ast::LiteralFloat("1.432");
    Term *float_t = lfloat->createTerm(branch);
    test_assert(float_t->type == FLOAT_TYPE);
    test_assert(as_float(float_t) > 1.431 && as_float(float_t) < 1.433);
    delete lfloat;

    ast::LiteralString *lstr = new ast::LiteralString("hello");
    Term *str_t = lstr->createTerm(branch);
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

    Term *term = functionCall->createTerm(branch);
    test_assert(as_function(term->function).name == "add");

    // make sure this term is not evaluated yet
    test_assert(term->needsUpdate);

    term->eval();

    test_assert(!term->needsUpdate);
    test_assert(as_float(term) == 5);

    delete functionCall;
}

void test_apply_statement()
{
    Branch branch;

    Term* result = apply_statement(branch, "something = add(5.0,3.0)");

    test_assert(result != NULL);
    test_assert(result->name == "something");
    test_assert(as_function(result->function).name == "add");
    test_assert(result->needsUpdate);
    evaluate_term(result);
    test_assert(as_float(result) == 8);
    test_assert(!result->needsUpdate);
}

void function_decl_ast()
{
    // Create a FunctionDecl from scratch
    ast::FunctionDecl decl;

    decl.header = new ast::FunctionHeader();
    decl.header->functionName = "add";
    decl.header->outputType = "float";
    decl.header->addArgument("float","arg1");
    decl.header->addArgument("float","arg2");

    decl.statements = new ast::StatementList();

    token_stream::TokenStream tokens("x = add(arg1,arg2)");
    decl.statements->push(parser::statement(tokens));

    test_assert(decl.toString() == "function add(float arg1, float arg2)\n{\nx = add(arg1,arg2)\n}");
}

void function_header()
{
    token_stream::TokenStream tokens("function test(int a, float, string b , int) -> float");
    ast::FunctionHeader* header = parser::functionHeader(tokens);

    test_assert(header->functionName == "test");
    test_assert(header->arguments[0].type == "int");
    test_assert(header->arguments[0].name == "a");
    test_assert(header->arguments[1].type == "float");
    test_assert(header->arguments[1].name == "");
    test_assert(header->arguments[2].type == "string");
    test_assert(header->arguments[2].name == "b");
    test_assert(header->arguments[3].type == "int");
    test_assert(header->arguments[3].name == "");
    test_assert(header->outputType == "float");

    delete header;
}

void function_decl_parse()
{
    token_stream::TokenStream tokens("function func(string a, int b) -> void\n{\n"
            "concat(a, to-string(b)) -> print\n"
            "}\n");
    ast::Statement* statement = parser::statement(tokens);

    test_assert(statement->typeName() == "function-decl");

    tokens.reset("function func2() {\n"
            "print(\"hello\")\n"
            "}\n"
            "some-function(1,2)\n");

    ast::StatementList* statementList = parser::statementList(tokens);
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

    Branch branch;
    Term* a = int_var(branch, 2, "a");
    Term* b = int_var(branch, 5, "b");

    test_assert(branch.getNamed("a") == a);
    test_assert(branch.getNamed("b") == b);

    Term* result = apply_statement(branch, "add(@a, b)");

    test_assert(branch.getNamed("a") == result);
    test_assert(branch.getNamed("b") == b);
}

void ast_walk()
{
    token_stream::TokenStream tokens("concat(to-string(add(1,2.0)), \"cheese\")");
    ast::Expression* expr = parser::infixExpression(tokens);

    std::vector<std::string> expected;
    expected.push_back("concat(to-string(add(1,2.0)), \"cheese\")");
    expected.push_back("to-string(add(1,2.0))");
    expected.push_back("add(1,2.0)");
    expected.push_back("1");
    expected.push_back("2.0");
    expected.push_back("\"cheese\"");

    struct MyWalker : public ast::ExpressionWalker
    {
        std::vector<std::string> found;

        virtual void visit(ast::Expression* expr)
        {
            found.push_back(expr->toString());
        }
    } walker;

    expr->walk(walker);

    for (int i=0; i < (int) expected.size(); i++) {
        test_assert(expected[i] == walker.found[i]);
    }

    delete expr;
}

void test_eval_statement()
{
    Branch b;

    test_assert(eval_statement(b, "5")->asInt() == 5);
    test_assert(eval_statement(b, "-2")->asInt() == -2);
    test_assert(eval_statement(b, "1.0")->asFloat() == 1.0);
    //test_assert(eval_statement(b, "-.123")->asFloat() == -0.123);FIXME
    test_assert(eval_statement(b, "\"apple\"")->asString() == "apple");
    test_assert(eval_statement(b, "\"\"")->asString() == "");
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
    REGISTER_TEST_CASE(parser_tests::test_apply_statement);
    REGISTER_TEST_CASE(parser_tests::function_decl_ast);
    REGISTER_TEST_CASE(parser_tests::function_header);
    REGISTER_TEST_CASE(parser_tests::function_decl_parse);
    REGISTER_TEST_CASE(parser_tests::rebind_operator);
    REGISTER_TEST_CASE(parser_tests::ast_walk);
    REGISTER_TEST_CASE(parser_tests::test_eval_statement);
}

} // namespace circa
