
#include "common_headers.h"

#include "tokenizer.h"
#include "parser.h"

namespace circa {
namespace parser {

ast::StatementList* statementList(token_stream::TokenStream& tokens)
{
    ast::StatementList* sl = new ast::StatementList();

    while (!tokens.finished()) {
        sl->push(statement(tokens));
    }

    return sl;
}

ast::Statement* statement(token_stream::TokenStream& tokens)
{
    ast::Statement* statement = new ast::Statement();

    // check for name binding
    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.nextIs(tokenizer::EQUALS, 1)) {
        statement->mNameBinding = tokens.consume(tokenizer::IDENTIFIER);
        tokens.consume(tokenizer::EQUALS);
    }

    statement->mExpression = infixExpression(tokens);

    return statement;
}

ast::Expression* infixExpression(token_stream::TokenStream& tokens)
{
    // Todo: handle infix expressions
    return atom(tokens);
}

ast::Expression* atom(token_stream::TokenStream& tokens)
{
    // function call?
    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.nextIs(tokenizer::LPAREN, 1))
        return functionCall(tokens);

    // literal string?
    if (tokens.nextIs(tokenizer::STRING) || tokens.nextIs(tokenizer::QUOTED_IDENTIFIER))
        return new ast::LiteralString(tokens.consume());

    // literal float?
    if (tokens.nextIs(tokenizer::FLOAT))
        return new ast::LiteralFloat(tokens.consume(tokenizer::FLOAT));

    // identifier?
    if (tokens.nextIs(tokenizer::IDENTIFIER))
        return new ast::Identifier(tokens.consume(tokenizer::IDENTIFIER));

    // parenthesized expression?
    if (tokens.nextIs(tokenizer::LPAREN)) {
        tokens.consume(tokenizer::LPAREN);
        ast::Expression* expr = infixExpression(tokens);
        tokens.consume(tokenizer::RPAREN);
        return expr;
    }
    
    return NULL;
}

ast::FunctionCall* functionCall(token_stream::TokenStream& tokens)
{
    return NULL;
}

ast::LiteralString* literalString(token_stream::TokenStream& tokens)
{
    return NULL;
}

ast::LiteralFloat* literalFloat(token_stream::TokenStream& tokens)
{
    return NULL;
}

ast::Identifier* identifier(token_stream::TokenStream& tokens)
{
    return NULL;
}

} // namespace parser
} // namespace circa
