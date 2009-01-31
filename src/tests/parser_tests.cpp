// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "testing.h"
#include "circa.h"
#include "compilation.h"

namespace circa {
namespace parser_tests {

void literal_float()
{
    TokenStream tokens("1.0");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralFloat *literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "1.0");
    test_assert(literal->hasQuestionMark == false);

    delete expr;

    tokens.reset("5.0?");
    expr = parser::atom(tokens);
    literal = dynamic_cast<ast::LiteralFloat*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "5.0");
    test_assert(literal->hasQuestionMark == true);

    delete expr;
}

void literal_string()
{
    TokenStream tokens("\"quoted string\"");
    ast::Expression *expr = parser::atom(tokens);
    ast::LiteralString *literal = dynamic_cast<ast::LiteralString*>(expr);
    test_assert(literal != NULL);
    test_assert(literal->text == "quoted string");

    delete expr;
}

void function_call()
{
    TokenStream tokens("add(1,2)");
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
    test_equals(print_ast(functionCall), "(FunctionCall LiteralInteger LiteralInteger)");
    
    delete functionCall;
}

void name_binding_expression()
{
    TokenStream tokens("name = hi(2,u)");
    ast::ExpressionStatement *statement = parser::expressionStatement(tokens);

    test_assert(statement->nameBinding == "name");

    ast::FunctionCall *functionCall =
        dynamic_cast<ast::FunctionCall*>(statement->expression);

    test_equals(functionCall->functionName, "hi");
    
    test_equals(ast::print_ast(functionCall),
            "(FunctionCall LiteralInteger Identifier)");

    delete statement;
}

void test_to_string()
{
    /*
    // Build an AST programatically so that the toString function
    // can't cheat.
    ast::FunctionCall *functionCall = new ast::FunctionCall("something");
    functionCall->addArgument(new ast::LiteralString("input0"), " ", "");
    functionCall->addArgument(new ast::LiteralInteger("4"), "", " ");
    test_assert(functionCall->toString() == "something( \"input0\",4 )");
    delete functionCall;
    */
}

void create_literals()
{
    Branch branch;
    CompilationContext context(&branch);

    ast::LiteralInteger *lint = new ast::LiteralInteger("13");
    Term *int_t = lint->createTerm(context);
    test_assert(int_t->type == INT_TYPE);
    test_assert(as_int(int_t) == 13);
    delete lint;

    ast::LiteralFloat *lfloat = new ast::LiteralFloat("1.432");
    Term *float_t = lfloat->createTerm(context);
    test_assert(float_t->type == FLOAT_TYPE);
    test_assert(as_float(float_t) > 1.431 && as_float(float_t) < 1.433);
    delete lfloat;

    ast::LiteralString *lstr = new ast::LiteralString("hello");
    Term *str_t = lstr->createTerm(context);
    test_assert(str_t->type == STRING_TYPE);
    test_assert(as_string(str_t) == "hello");
    delete lstr;
}

void create_function_call()
{
    Branch branch;
    CompilationContext context(&branch);

    ast::FunctionCall *functionCall = new ast::FunctionCall("add");
    functionCall->addArgument(new ast::LiteralFloat("2"), "", "");
    functionCall->addArgument(new ast::LiteralFloat("3"), "", "");

    Term *term = functionCall->createTerm(context);
    test_assert(as_function(term->function).name == "add");

    // make sure this term is not evaluated yet
    test_assert(term->needsUpdate);

    evaluate_term(term);

    test_assert(!term->needsUpdate);
    test_assert(as_float(term) == 5);

    delete functionCall;
}

void test_compile_statement()
{
    Branch branch;

    Term* result = compile_statement(&branch, "something = add(5.0,3.0)");

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

    TokenStream tokens("x = add(arg1,arg2)");
    decl.statements->push(parser::statement(tokens));

    test_equals(ast::print_ast(&decl),
            "(FunctionDecl (StatementList ExpressionStatement))");
}

void function_header()
{
    ast::FunctionHeader header = eval_as<ast::FunctionHeader>(
        "'function test(int a, float, string b , int) -> float'.tokenize.parse-function-header");

    test_assert(header.functionName == "test");
    test_assert(header.arguments[0].type == "int");
    test_assert(header.arguments[0].name == "a");
    test_assert(header.arguments[1].type == "float");
    test_assert(header.arguments[1].name == "");
    test_assert(header.arguments[2].type == "string");
    test_assert(header.arguments[2].name == "b");
    test_assert(header.arguments[3].type == "int");
    test_assert(header.arguments[3].name == "");
    test_assert(header.outputType == "float");
}

void function_decl_parse()
{
    TokenStream tokens("function func(string a, int b) -> void\n"
            "concat(a, to-string(b)) -> print\n"
            "end");
    ast::Statement* statement = parser::statement(tokens);

    test_assert(statement->typeName() == "FunctionDecl");

    tokens.reset("function func2()\n"
            "print(\"hello\")\n"
            "end\n"
            "some-function(1,2)\n");

    ast::StatementList* statementList = parser::statementList(tokens);

    delete statement;
    delete statementList;
}

void rebind_operator()
{
    TokenStream tokens("@cheese");
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
    Term* a = int_value(branch, 2, "a");
    Term* b = int_value(branch, 5, "b");

    test_assert(branch.getNamed("a") == a);
    test_assert(branch.getNamed("b") == b);

    Term* result = compile_statement(&branch, "add(@a, b)");

    test_assert(branch.getNamed("a") == result);
    test_assert(branch.getNamed("b") == b);
}

void test_eval_statement()
{
    Branch b;

    test_assert(eval_statement(&b, "5")->asInt() == 5);
    test_assert(eval_statement(&b, "-2")->asInt() == -2);
    test_assert(eval_statement(&b, "1.0")->asFloat() == 1.0);
    //test_assert(eval_statement(&b, "-.123")->asFloat() == -0.123);FIXME
    test_assert(eval_statement(&b, "\"apple\"")->asString() == "apple");
    test_assert(eval_statement(&b, "\"\"")->asString() == "");
}

void comment_statement()
{
    TokenStream tokens("-- this is a comment");

    ast::Statement* statement = parser::statement(tokens);
    test_assert(dynamic_cast<ast::CommentStatement*>(statement) != NULL);

    test_assert(tokens.finished());

    delete statement;
}

void test_type_decl()
{
    TokenStream tokens("type Vector {\nfloat x\nint y\n}");

    ast::TypeDecl* typeDecl = parser::typeDecl(tokens);
    test_assert(typeDecl != NULL);
    test_assert(typeDecl->name == "Vector");
    test_assert(typeDecl->members[0].type == "float");
    test_assert(typeDecl->members[0].name == "x");
    test_assert(typeDecl->members[1].type == "int");
    test_assert(typeDecl->members[1].name == "y");

    delete typeDecl;
}

void test_type_decl_statement()
{
    TokenStream tokens("type Mytype\n{\nstring str\nfloat asdf\n}");
    ast::Statement* statement = parser::statement(tokens);

    test_assert(dynamic_cast<ast::TypeDecl*>(statement) != NULL);

    delete statement;
}

void test_type_decl_full_trip()
{
    Branch branch;
    Term *t = eval_statement(&branch, "type Mytype\n{\nstring s\nfloat a\n}");

    test_assert(!t->hasError());

    test_assert(t->name == "Mytype");
    test_assert(as_type(t).fields[0].type == STRING_TYPE);
    test_assert(as_type(t).fields[0].name == "s");
    test_assert(as_type(t).fields[1].type == FLOAT_TYPE);
    test_assert(as_type(t).fields[1].name == "a");
}

void test_parse_if_block()
{
    TokenStream tokens(
            "if (x > 2) \n print('yes') \n else \n print('no') \n end");

    ast::IfStatement* ifStatement = parser::ifStatement(tokens);

    test_assert(ifStatement != NULL);

    // these lines will need to be fixed when there is better whitespace preservation
    test_equals(ast::print_ast(ifStatement->condition),
            "(Infix Identifier LiteralInteger)");
    test_equals(ast::print_ast(ifStatement->positiveBranch),
            "(StatementList ExpressionStatement)");
    test_equals(ast::print_ast(ifStatement->negativeBranch),
            "(StatementList ExpressionStatement)");

    delete ifStatement;

    tokens.reset("if (false) \n print('blah') \n end");

    ifStatement = parser::ifStatement(tokens);

    test_equals(ast::print_ast(ifStatement->condition), "Identifier");
    test_equals(ast::print_ast(ifStatement->positiveBranch),
            "(StatementList ExpressionStatement)");
    test_assert(ifStatement->negativeBranch == NULL);

    delete ifStatement;
}

void member_function_call_parse()
{
    TokenStream tokens("thing.func(1)");
    ast::Expression* expression = parser::infixExpression(tokens);

    test_assert(expression != NULL);

    test_assert(ast::print_ast(expression) == 
            "(Infix Identifier (FunctionCall LiteralInteger))");
}

void stateful_value_declaration_test()
{
    TokenStream tokens("state int x");
    ast::StatefulValueDeclaration *svd = parser::statefulValueDeclaration(tokens);
    test_assert(svd != NULL);
    test_assert(svd->type == "int");
    test_assert(svd->name == "x");
    test_assert(svd->initialValue == NULL);
    delete svd;

    tokens.reset("state string blah = \"hey\"");
    svd = parser::statefulValueDeclaration(tokens);
    test_assert(svd != NULL);
    test_assert(svd->type == "string");
    test_assert(svd->name == "blah");
    test_assert(svd->initialValue != NULL);
    test_equals(ast::print_ast(svd->initialValue), "LiteralString");
    test_equals(ast::print_ast(svd), "(StatefulValueDeclaration LiteralString)");
    delete svd;
}

void register_tests()
{
    REGISTER_TEST_CASE(parser_tests::literal_float);
    REGISTER_TEST_CASE(parser_tests::literal_string);
    REGISTER_TEST_CASE(parser_tests::function_call);
    REGISTER_TEST_CASE(parser_tests::name_binding_expression);
    REGISTER_TEST_CASE(parser_tests::test_to_string);
    REGISTER_TEST_CASE(parser_tests::create_literals);
    REGISTER_TEST_CASE(parser_tests::create_function_call);
    REGISTER_TEST_CASE(parser_tests::test_compile_statement);
    REGISTER_TEST_CASE(parser_tests::function_decl_ast);
    REGISTER_TEST_CASE(parser_tests::function_header);
    REGISTER_TEST_CASE(parser_tests::function_decl_parse);
    REGISTER_TEST_CASE(parser_tests::rebind_operator);
    REGISTER_TEST_CASE(parser_tests::test_eval_statement);
    REGISTER_TEST_CASE(parser_tests::comment_statement);
    REGISTER_TEST_CASE(parser_tests::test_type_decl);
    REGISTER_TEST_CASE(parser_tests::test_type_decl_statement);
    REGISTER_TEST_CASE(parser_tests::test_type_decl_full_trip);
    REGISTER_TEST_CASE(parser_tests::test_parse_if_block);
    REGISTER_TEST_CASE(parser_tests::member_function_call_parse);
    REGISTER_TEST_CASE(parser_tests::stateful_value_declaration_test);
}

} // namespace parser_tests
} // namespace circa
