
#include "common_headers.h"

#include "tokenizer.h"
#include "parser.h"

namespace circa {
namespace parser {

std::string possibleWhitespace(token_stream::TokenStream& tokens);

ast::StatementList* statementList(token_stream::TokenStream& tokens)
{
    ast::StatementList* sl = new ast::StatementList();

    while (!tokens.finished()) {
        if (tokens.nextIs(tokenizer::NEWLINE)) {
            tokens.consume(tokenizer::NEWLINE);
            continue;
        }

        sl->push(statement(tokens));
    }

    return sl;
}

ast::Statement* statement(token_stream::TokenStream& tokens)
{
    ast::Statement* statement = new ast::Statement();

    bool isNameBinding = tokens.nextIs(tokenizer::IDENTIFIER)
        && (tokens.nextIs(tokenizer::EQUALS, 1)
                || (tokens.nextIs(tokenizer::WHITESPACE,1)
                    && (tokens.nextIs(tokenizer::EQUALS,2))));

    // check for name binding
    if (isNameBinding) {
        statement->nameBinding = tokens.consume(tokenizer::IDENTIFIER);
        statement->preEqualsWhitepace = possibleWhitespace(tokens);
        tokens.consume(tokenizer::EQUALS);
        statement->postEqualsWhitespace = possibleWhitespace(tokens);
    }

    statement->expression = infixExpression(tokens);

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

    // literal integer?
    if (tokens.nextIs(tokenizer::INTEGER))
        return new ast::LiteralInteger(tokens.consume(tokenizer::INTEGER));

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

    throw syntax_errors::UnrecognizedExpression();
}

ast::FunctionCall* functionCall(token_stream::TokenStream& tokens)
{
    std::string functionName = tokens.consume(tokenizer::IDENTIFIER);
    tokens.consume(tokenizer::LPAREN);

    std::auto_ptr<ast::FunctionCall> functionCall(new ast::FunctionCall(functionName));

    while (!tokens.nextIs(tokenizer::RPAREN))
    {
        std::string preWhitespace = possibleWhitespace(tokens);
        std::auto_ptr<ast::Expression> expression(infixExpression(tokens));
        std::string postWhitespace = possibleWhitespace(tokens);

        functionCall->addArgument(expression.release(), preWhitespace, postWhitespace);

        if (!tokens.nextIs(tokenizer::RPAREN))
            tokens.consume(tokenizer::COMMA);
    }
    
    return functionCall.release();
}

std::string possibleWhitespace(token_stream::TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::WHITESPACE))
        return tokens.consume(tokenizer::WHITESPACE);
    else
        return "";
}

} // namespace parser
} // namespace circa
