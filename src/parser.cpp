// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "parser.h"
#include "pointer_visitor.h"
#include "runtime.h"
#include "tokenizer.h"

namespace circa {

Term* apply_statement(Branch& branch, std::string const& input)
{
    token_stream::TokenStream tokens(input);

    ast::Statement* statementAst = parser::statement(tokens);
    assert(statementAst != NULL);

    Term* result = statementAst->createTerm(branch);
    assert(result != NULL);

    // check for rebind operator
    struct RebindFinder : public ast::ExpressionWalker
    {
        std::vector<std::string> rebindNames;

        virtual void visit(ast::Expression* expr)
        {
            ast::Identifier* asIdent = dynamic_cast<ast::Identifier*>(expr);
            if (asIdent == NULL)
                return;

            if (asIdent->hasRebindOperator)
                rebindNames.push_back(asIdent->text);
        }

    } rebindFinder;

    if (statementAst->expression != NULL) {
        statementAst->expression->walk(rebindFinder);

        std::vector<std::string>::iterator it;
        for (it = rebindFinder.rebindNames.begin(); it != rebindFinder.rebindNames.end(); ++it)
        {
            branch.bindName(result, *it);
        }
    }

    delete statementAst;

    return result;
}

Term* eval_statement(Branch& branch, std::string const& input)
{
    Term* term = apply_statement(branch, input);
    evaluate_term(term);
    return term;
}

namespace parser {

std::string possibleWhitespace(token_stream::TokenStream& tokens);
std::string possibleNewline(token_stream::TokenStream& tokens);

ast::StatementList* statementList(token_stream::TokenStream& tokens)
{
    ast::StatementList* statementList = new ast::StatementList();

    while (!tokens.finished()) {
        if (tokens.nextIs(tokenizer::NEWLINE)) {
            tokens.consume(tokenizer::NEWLINE);
            continue;
        }

        if (tokens.nextIs(tokenizer::RBRACE) || tokens.nextIs(tokenizer::END)) {
            break;
        }

        statementList->push(statement(tokens));
    }

    return statementList;
}

ast::Statement* statement(token_stream::TokenStream& tokens)
{
    // toss leading whitespace
    possibleWhitespace(tokens);

    // Check for comment line
    if (tokens.nextIs(tokenizer::DOUBLE_MINUS)) {
        tokens.consume(tokenizer::DOUBLE_MINUS);

        // Throw away the rest of this line
        while (!tokens.finished()) {
            if (tokens.nextIs(tokenizer::NEWLINE)) {
                tokens.consume(tokenizer::NEWLINE);
                break;
            }

            tokens.consume();
        }

        return new ast::IgnorableStatement();
    }

    // Check for blank line
    if (tokens.nextIs(tokenizer::NEWLINE) || tokens.finished()) {
        if (tokens.nextIs(tokenizer::NEWLINE))
            tokens.consume(tokenizer::NEWLINE);
        return new ast::IgnorableStatement();
    }

    // Check for keywords
    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "function") {
        return functionDecl(tokens);
    }

    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "type") {
        return typeDecl(tokens);
    }

    return expressionStatement(tokens);
}

ast::ExpressionStatement* expressionStatement(token_stream::TokenStream& tokens)
{
    ast::ExpressionStatement* statement = new ast::ExpressionStatement();

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

    // check for return statement
    else if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "return") {
        tokens.consume(tokenizer::IDENTIFIER);
        statement->nameBinding = "#output";
        statement->preEqualsWhitepace = "";
        statement->postEqualsWhitespace = possibleWhitespace(tokens);
    }

    statement->expression = infixExpression(tokens);

    return statement;
}

int getInfixPrecedence(int match)
{
    switch(match) {
        case tokenizer::DOT:
            return 8;
        case tokenizer::STAR:
        case tokenizer::SLASH:
            return 7;
        case tokenizer::PLUS:
        case tokenizer::MINUS:
            return 6;
        case tokenizer::LTHAN:
        case tokenizer::LTHANEQ:
        case tokenizer::GTHAN:
        case tokenizer::GTHANEQ:
        case tokenizer::DOUBLE_EQUALS:
        case tokenizer::NOT_EQUALS:
            return 5;
        case tokenizer::DOUBLE_AMPERSAND:
        case tokenizer::DOUBLE_VERTICAL_BAR:
            return 4;
        case tokenizer::EQUALS:
        case tokenizer::PLUS_EQUALS:
        case tokenizer::MINUS_EQUALS:
        case tokenizer::STAR_EQUALS:
        case tokenizer::SLASH_EQUALS:
            return 2;
        case tokenizer::RIGHT_ARROW:
            return 1;
        case tokenizer::COLON_EQUALS:
            return 0;
        default:
            return -1;
    }
}

#define HIGHEST_INFIX_PRECEDENCE 8

ast::Expression* infixExpression(token_stream::TokenStream& tokens,
        int precedence)
{
    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return atom(tokens);

    ast::Expression* leftExpr = infixExpression(tokens, precedence+1);

    possibleWhitespace(tokens);

    while (!tokens.finished() && getInfixPrecedence(tokens.next().match) == precedence) {

        std::string operatorStr = tokens.consume();

        possibleWhitespace(tokens);

        ast::Expression* rightExpr = infixExpression(tokens, precedence+1);

        leftExpr = new ast::Infix(operatorStr, leftExpr, rightExpr);
    }

    return leftExpr;
}

void post_literal(token_stream::TokenStream& tokens, ast::Literal* literal)
{
    if (tokens.nextIs(tokenizer::QUESTION)) {
        tokens.consume(tokenizer::QUESTION);
        literal->hasQuestionMark = true;
    }
}

ast::Literal* literal_float(token_stream::TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralFloat(tokens.consume(tokenizer::FLOAT));
    post_literal(tokens, lit);
    return lit;
}

ast::Literal* literal_integer(token_stream::TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralInteger(tokens.consume(tokenizer::INTEGER));
    post_literal(tokens, lit);
    return lit;
}

ast::Literal* literal_string(token_stream::TokenStream& tokens)
{
    ast::Literal* lit = new ast::LiteralString(tokens.consume());
    post_literal(tokens, lit);
    return lit;
}

ast::Expression* atom(token_stream::TokenStream& tokens)
{
    // function call?
    if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.nextIs(tokenizer::LPAREN, 1))
        return functionCall(tokens);

    // literal string?
    if (tokens.nextIs(tokenizer::STRING))
        return literal_string(tokens);

    // literal float?
    if (tokens.nextIs(tokenizer::FLOAT))
        return literal_float(tokens);

    // literal integer?
    if (tokens.nextIs(tokenizer::INTEGER))
        return literal_integer(tokens);

    // rebind operator?
    bool hasRebindOperator = false;

    if (tokens.nextIs(tokenizer::AMPERSAND)) {
        hasRebindOperator = true;
        tokens.consume(tokenizer::AMPERSAND);
    }

    // identifier?
    if (tokens.nextIs(tokenizer::IDENTIFIER)) {
        ast::Identifier* id = new ast::Identifier(tokens.consume(tokenizer::IDENTIFIER));
        id->hasRebindOperator = hasRebindOperator;
        return id;
    } else if (hasRebindOperator) {
        syntax_error("@ operator only allowed before an identifier");
    }

    // parenthesized expression?
    if (tokens.nextIs(tokenizer::LPAREN)) {
        tokens.consume(tokenizer::LPAREN);
        ast::Expression* expr = infixExpression(tokens);
        tokens.consume(tokenizer::RPAREN);
        return expr;
    }

    syntax_error("Unrecognized expression", &tokens.next());

    return NULL; // unreachable
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

    tokens.consume(tokenizer::RPAREN);
    
    return functionCall.release();
}

ast::FunctionHeader* functionHeader(token_stream::TokenStream& tokens)
{
    std::auto_ptr<ast::FunctionHeader> header(new ast::FunctionHeader());

    std::string firstIdentifier = tokens.consume(tokenizer::IDENTIFIER); // 'function'
    possibleWhitespace(tokens);

    if (firstIdentifier == "function") {
        header->functionName = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);
    } else {
        header->functionName = firstIdentifier;
    }

    tokens.consume(tokenizer::LPAREN);

    while (!tokens.nextIs(tokenizer::RPAREN))
    {
        std::string preWhitespace = possibleWhitespace(tokens);
        std::string type = tokens.consume(tokenizer::IDENTIFIER);
        std::string innerWhitespace = possibleWhitespace(tokens);

        std::string name, postWhitespace;
        if (tokens.nextIs(tokenizer::COMMA) || tokens.nextIs(tokenizer::RPAREN)) {
            name = "";
            postWhitespace = "";
        } else {
            name = tokens.consume(tokenizer::IDENTIFIER);
            postWhitespace = possibleWhitespace(tokens);
        }

        header->addArgument(type, name);

        if (!tokens.nextIs(tokenizer::RPAREN))
            tokens.consume(tokenizer::COMMA);
    }

    tokens.consume(tokenizer::RPAREN);

    possibleWhitespace(tokens);

    if (tokens.nextIs(tokenizer::RIGHT_ARROW)) {
        tokens.consume(tokenizer::RIGHT_ARROW);
        possibleWhitespace(tokens);
        header->outputType = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);
    }

    return header.release();
}

ast::FunctionDecl* functionDecl(token_stream::TokenStream& tokens)
{
    std::auto_ptr<ast::FunctionDecl> decl(new ast::FunctionDecl());

    decl->header = functionHeader(tokens);

    possibleNewline(tokens);

    /*tokens.consume(tokenizer::LBRACE);

    possibleNewline(tokens);*/

    decl->statements = statementList(tokens);

    possibleNewline(tokens);

    tokens.consume(tokenizer::END);

    possibleNewline(tokens);
    
    return decl.release();
}

ast::TypeDecl* typeDecl(token_stream::TokenStream& tokens)
{
    std::auto_ptr<ast::TypeDecl> decl(new ast::TypeDecl());

    std::string typeKeyword = tokens.consume(tokenizer::IDENTIFIER);
    assert(typeKeyword == "type");

    possibleWhitespace(tokens);

    decl->name = tokens.consume(tokenizer::IDENTIFIER);

    possibleNewline(tokens);

    tokens.consume(tokenizer::LBRACE);

    possibleNewline(tokens);

    while (true) {

        possibleWhitespace(tokens);

        if (tokens.nextIs(tokenizer::RBRACE)) {
            tokens.consume(tokenizer::RBRACE);
            break;
        }

        std::string fieldType = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);
        std::string fieldName = tokens.consume(tokenizer::IDENTIFIER);
        possibleWhitespace(tokens);

        decl->addMember(fieldType, fieldName);

        tokens.consume(tokenizer::NEWLINE);
    }

    return decl.release();
}

std::string possibleWhitespace(token_stream::TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::WHITESPACE))
        return tokens.consume(tokenizer::WHITESPACE);
    else
        return "";
}

std::string possibleNewline(token_stream::TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(tokenizer::NEWLINE) || tokens.nextIs(tokenizer::WHITESPACE))
        output << tokens.consume();

    return output.str();
}

void syntax_error(std::string const& message, tokenizer::TokenInstance const* location)
{
    std::stringstream out;
    out << "Syntax error: " << message;

    if (location != NULL) {
        out << ", at line " << location->line << ", char " << location->character;
    }

    throw std::runtime_error(out.str());
}

} // namespace parser
} // namespace circa
