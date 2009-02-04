// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "compilation.h"
#include "function.h"
#include "parser.h"
#include "pointer_visitor.h"
#include "runtime.h"
#include "tokenizer.h"

/*
namespace circa {
namespace newparser {

Term* compile_statement(Branch& branch, std::string const& input)
{
    TokenStream tokens(input);

    return statement(branch, input);
}

Term* statement(Branch& branch, TokenStream& tokens)
{
    std::string precedingWhitespace = possible_whitespace(tokens);

    Term* result = NULL;

    // Check for comment line
    if (tokens.nextIs(tokenizer::DOUBLE_MINUS)) {
        result = comment_statement(branch, tokens);
    }

    // Check for blank line
    else if (tokens.nextIs(tokenizer::NEWLINE)) {
        result = blank_line(branch, tokens);
    }

    // Check for keywords
    else if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "function") {
        result = functionDecl(tokens);
    }

    else if (tokens.nextIs(tokenizer::IDENTIFIER) && tokens.next().text == "type") {
        result = typeDecl(tokens);
    }

    else if (tokens.nextIs(tokenizer::IF)) {
        result = ifStatement(tokens);
    }

    else if (tokens.nextIs(tokenizer::STATE)) {
        result = statefulValueDeclaration(tokens);
    }

    else {
        result = expressionStatement(tokens);
    }
}

Term* comment_statement(Branch& branch, TokenStream& tokens)
{
    std::stringstream commentText;
    tokens.consume(tokenizer::DOUBLE_MINUS);

    // Throw away the rest of this line
    while (!tokens.finished()) {
        if (tokens.nextIs(tokenizer::NEWLINE)) {
            tokens.consume(tokenizer::NEWLINE);
            break;
        }

        commentText << tokens.next().text;
        tokens.consume();
    }

    Term* result = apply_function(&branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = commentText;
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    result->syntaxHints.occursInsideAnExpression = false;
    return result;
}

Term* blank_line(Branch& branch, TokenStream& tokens)
{

}

std::string possible_whitespace(TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::WHITESPACE))
        return tokens.consume(tokenizer::WHITESPACE);
    else
        return "";
}

std::string possible_newline(TokenStream& tokens)
{
    if (tokens.nextIs(tokenizer::NEWLINE))
        return tokens.consume(tokenizer::NEWLINE);
    else
        return "";
}

std::string possible_whitespace_or_newline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(tokenizer::NEWLINE) || tokens.nextIs(tokenizer::WHITESPACE))
        output << tokens.consume();

    return output.str();
}

} // namespace parser
} // namespace circa
*/
