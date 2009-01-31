// Copyright 2008 Andrew Fischer

#ifndef CIRCA_PARSER_INCLUDED
#define CIRCA_PARSER_INCLUDED

#include "ast.h"
#include "token_stream.h"

namespace circa {

Term* compile_statement(Branch* branch, std::string const& input);
Term* eval_statement(Branch* branch, std::string const& input);

namespace parser {

std::string possibleWhitespace(TokenStream& tokens);
ast::StatementList* statementList(TokenStream& tokens);
ast::Statement* statement(TokenStream& tokens);
ast::ExpressionStatement* expressionStatement(TokenStream& tokens);
ast::Expression* infixExpression(TokenStream& tokens,
        int precedence=0);
ast::Expression* atom(TokenStream& tokens);
ast::FunctionCall* functionCall(TokenStream& tokens);
ast::LiteralString* literalString(TokenStream& tokens);
ast::LiteralFloat* literalFloat(TokenStream& tokens);
ast::Identifier* identifier(TokenStream& tokens);
ast::FunctionHeader* functionHeader(TokenStream& tokens);
ast::FunctionDecl* functionDecl(TokenStream& tokens);
ast::TypeDecl* typeDecl(TokenStream& tokens);
ast::IfStatement* ifStatement(TokenStream& tokens);
ast::StatefulValueDeclaration* statefulValueDeclaration(TokenStream& tokens);

void syntax_error(std::string const& message,
        tokenizer::TokenInstance const* location = NULL);

} // namespace parser
} // namespace circa

#endif
