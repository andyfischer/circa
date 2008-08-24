
#include "common_headers.h"

#include "token.h"
#include "parser.h"

namespace circa {
namespace parser {

ast::StatementList* statementList(TokenStream* tokens)
{
    ast::StatementList* sl = new ast::StatementList();

    while (!tokens->finished()) {
        sl->push(statement(tokens));
    }

    return sl;
}

ast::Statement* statement(TokenStream* tokens)
{
    ast::Statement* statement = new ast::Statement();

    // check for name binding
    if (tokens->nextIs(token::IDENTIFIER) && tokens->nextIs(token::EQUALS, 1)) {
        statement->mNameBinding = tokens->consume(token::IDENTIFIER);
        tokens->consume(token::EQUALS);
    }

    statement->mExpression = infixExpression(tokens);

    return statement;
}

ast::Expression* infixExpression(TokenStream* tokens)
{
    // Todo: handle infix expressions
    return atom(tokens);
}

ast::Expression* atom(TokenStream* tokens)
{
    // function call?
    if (tokens->nextIs(token::IDENTIFIER) && tokens->nextIs(token::LPAREN, 1))
        return functionCall(tokens);

    // literal string?
    if (tokens->nextIs(token::STRING) || tokens->nextIs(token::QUOTED_IDENTIFIER))
        return new ast::LiteralString(tokens->consume());

    // literal float?
    if (tokens->nextIs(token::FLOAT))
        return new ast::LiteralFloat(tokens->consume(token::FLOAT));

    // identifier?
    if (tokens->nextIs(token::IDENTIFIER))
        return new ast::Identifier(tokens->consume(token::IDENTIFIER));

    // parenthesized expression?
    if (tokens->nextIs(token::LPAREN)) {
        tokens->consume(token::LPAREN);
        ast::Expression* expr = infixExpression(tokens);
        tokens->consume(token::RPAREN);
        return expr;
    }
    
    return NULL;
}

ast::FunctionCall* functionCall(TokenStream* tokens)
{
    return NULL;
}

ast::LiteralString* literalString(TokenStream* tokens)
{
    return NULL;
}

ast::LiteralFloat* literalFloat(TokenStream* tokens)
{
    return NULL;
}

ast::Identifier* identifier(TokenStream* tokens)
{
    return NULL;
}

} // namespace parser
} // namespace circa
