// Copyright 2008 Andrew Fischer

#ifndef CIRCA_PARSER_INCLUDED
#define CIRCA_PARSER_INCLUDED

#include "ast.h"
#include "token_stream.h"

namespace circa {

Term* apply_statement(Branch& branch, std::string const& input);
Term* eval_statement(Branch& branch, std::string const& input);

namespace parser {

ast::StatementList* statementList(token_stream::TokenStream& tokens);
ast::Statement* statement(token_stream::TokenStream& tokens);
ast::ExpressionStatement* expressionStatement(token_stream::TokenStream& tokens);
ast::Expression* infixExpression(token_stream::TokenStream& tokens,
        int precedence=0);
ast::Expression* atom(token_stream::TokenStream& tokens);
ast::FunctionCall* functionCall(token_stream::TokenStream& tokens);
ast::LiteralString* literalString(token_stream::TokenStream& tokens);
ast::LiteralFloat* literalFloat(token_stream::TokenStream& tokens);
ast::Identifier* identifier(token_stream::TokenStream& tokens);
ast::FunctionHeader* functionHeader(token_stream::TokenStream& tokens);
ast::FunctionDecl* functionDecl(token_stream::TokenStream& tokens);
ast::TypeDecl* typeDecl(token_stream::TokenStream& tokens);

void syntax_error(std::string const& message,
        tokenizer::TokenInstance const* location = NULL);

} // namespace parser
} // namespace circa

#endif
