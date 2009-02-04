// Copyright 2008 Paul Hodge

#include "common_headers.h"

#include "ast.h"
#include "branch.h"
#include "builtins.h"
#include "compilation.h"
#include "function.h"
#include "parser.h"
#include "pointer_visitor.h"
#include "runtime.h"
#include "tokenizer.h"
#include "type.h"
#include "values.h"

#include "newparser.h"

namespace circa {
namespace newparser {

using namespace circa::tokenizer;

const int HIGHEST_INFIX_PRECEDENCE = 8;

Term* compile(Branch& branch, ParsingStep step, std::string const& input)
{
    TokenStream tokens(input);
    Term* result = step(branch, tokens);
    remove_compilation_attrs(branch);
    return result;
}

Term* compile_statement(Branch& branch, std::string const& input)
{
    TokenStream tokens(input);

    return statement(branch, tokens);
}

Term* statement(Branch& branch, TokenStream& tokens)
{
    std::string precedingWhitespace = possible_whitespace(tokens);

    Term* result = NULL;

    // Comment line
    if (tokens.nextIs(DOUBLE_MINUS)) {
        result = comment_statement(branch, tokens);
    }

    // Blank line
    else if (tokens.finished() || tokens.nextIs(NEWLINE)) {
        result = blank_line(branch, tokens);
    }

    // Function decl
    else if (tokens.nextIs(FUNCTION)) {
        result = function_decl(branch, tokens);
    }

    // Type decl
    else if (tokens.nextIs(TYPE)) {
        result = type_decl(branch, tokens);
    }

    // If statement
    else if (tokens.nextIs(IF)) {
        result = if_statement(branch, tokens);
    }

    // Stateful value decl
    else if (tokens.nextIs(STATE)) {
        result = stateful_value_decl(branch, tokens);
    }

    // Expression statement
    else {
        result = expression_statement(branch, tokens);
    }

    return result;
}

Term* comment_statement(Branch& branch, TokenStream& tokens)
{
    std::stringstream commentText;
    tokens.consume(DOUBLE_MINUS);

    // Throw away the rest of this line
    while (!tokens.finished()) {
        if (tokens.nextIs(NEWLINE)) {
            tokens.consume(NEWLINE);
            break;
        }

        commentText << tokens.next().text;
        tokens.consume();
    }

    Term* result = apply_function(&branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = commentText.str();
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    result->syntaxHints.occursInsideAnExpression = false;
    return result;
}

Term* blank_line(Branch& branch, TokenStream& tokens)
{
    if (!tokens.finished())
        tokens.consume(NEWLINE);

    Term* result = apply_function(&branch, COMMENT_FUNC, ReferenceList());
    as_string(result->state->field(0)) = "";
    result->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    result->syntaxHints.occursInsideAnExpression = false;

    return result;
}

Term* function_decl(Branch& branch, TokenStream& tokens)
{
    tokens.consume(FUNCTION);
    possible_whitespace(tokens);
    std::string functionName = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);
    tokens.consume(LPAREN);

    Term* result = create_value(&branch, FUNCTION_TYPE, functionName);
    Function& func = as_function(result);

    func.name = functionName;

    while (!tokens.nextIs(RPAREN))
    {
        possible_whitespace(tokens);
        std::string type = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);

        std::string name;
        
        if (tokens.nextIs(IDENTIFIER)) {
            name = tokens.consume(IDENTIFIER);
            possible_whitespace(tokens);
        }

        Term* typeTerm = find_named(&branch, type);

        if (typeTerm == NULL)
            throw std::runtime_error("couldn't find type: " + type);

        func.appendInput(typeTerm, name);

        if (!tokens.nextIs(RPAREN))
            tokens.consume(COMMA);
    }

    tokens.consume(RPAREN);

    possible_whitespace(tokens);

    if (tokens.nextIs(RIGHT_ARROW)) {
        tokens.consume(RIGHT_ARROW);
        possible_whitespace(tokens);
        std::string outputTypeName = tokens.consume(IDENTIFIER);
        Term* outputType = find_named(&branch, outputTypeName);

        if (outputType == NULL)
            throw std::runtime_error("couldn't find type: " + outputTypeName);

        func.outputType = outputType;
    }

    possible_whitespace_or_newline(tokens);

    while (!tokens.nextIs(END)) {
        statement(func.subroutineBranch, tokens);
    }

    return result;
}

Term* type_decl(Branch& branch, TokenStream& tokens)
{
    tokens.consume(TYPE);
    possible_whitespace(tokens);

    std::string name = tokens.consume(IDENTIFIER);

    Term* result = create_value(&branch, TYPE_TYPE, name);
    Type& type = as_type(result);

    type.name = name;

    possible_whitespace_or_newline(tokens);

    tokens.consume(LBRACE);

    while (!tokens.nextIs(RBRACE)) {
        possible_whitespace_or_newline(tokens);

        if (tokens.nextIs(RBRACE))
            break;

        std::string fieldTypeName = tokens.consume(IDENTIFIER);
        possible_whitespace(tokens);
        std::string fieldName = tokens.consume(IDENTIFIER);
        possible_whitespace_or_newline(tokens);

        Term* fieldType = find_named(&branch, fieldTypeName);

        if (fieldType == NULL)
            throw std::runtime_error("couldn't find type: " + fieldTypeName);

        type.addField(fieldType, fieldName);
    }

    return result;
}

Term* if_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(IF);
    possible_whitespace(tokens);

    Term* condition = infix_expression(branch, tokens);
    assert(condition != NULL);

    possible_whitespace_or_newline(tokens);

    Term* result = apply_function(&branch, "if-statement", ReferenceList(condition));
    Branch& posBranch = as_branch(result->state->field(0));
    Branch& negBranch = as_branch(result->state->field(1));
    Branch& joiningTermsBranch = as_branch(result->state->field(1));

    posBranch.outerScope = &branch;
    negBranch.outerScope = &branch;
    joiningTermsBranch.outerScope = &branch;

    while (!tokens.nextIs(ELSE) && !tokens.nextIs(END)) {
        statement(posBranch, tokens);

        if (tokens.nextIs(ELSE)) {
            tokens.consume(ELSE);
            while (!tokens.nextIs(END)) {
                statement(negBranch, tokens);
            }
            break;
        }
    }

    remove_compilation_attrs(posBranch);
    remove_compilation_attrs(negBranch);

    tokens.consume(END);

    possible_whitespace_or_newline(tokens);

    // TODO: joining terms
    return result;
}

Term* stateful_value_decl(Branch& branch, TokenStream& tokens)
{
    // TODO
    return NULL;
}

Term* expression_statement(Branch& branch, TokenStream& tokens)
{
    return infix_expression(branch, tokens);
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

std::string getInfixFunctionName(std::string const& infix)
{
    if (infix == "+")
        return "add";
    else if (infix == "-")
        return "sub";
    else if (infix == "*")
        return "mult";
    else if (infix == "/")
        return "div";
    else if (infix == "<")
        return "less-than";
    else if (infix == "<=")
        return "less-than-eq";
    else if (infix == ">")
        return "greater-than";
    else if (infix == ">=")
        return "greater-than-eq";
    else if (infix == "==")
        return "equals";
    else if (infix == "||")
        return "or";
    else if (infix == "&&")
        return "and";
    else
        return "#unrecognized";
}

Term* infix_expression(Branch& branch, TokenStream& tokens)
{
    return infix_expression_nested(branch, tokens, 0);
}

Term* infix_expression_nested(Branch& branch, TokenStream& tokens, int precedence)
{
    if (precedence > HIGHEST_INFIX_PRECEDENCE)
        return atom(branch, tokens);

    Term* leftExpr = infix_expression_nested(branch, tokens, precedence+1);

    possible_whitespace(tokens);

    while (!tokens.finished() && getInfixPrecedence(tokens.next().match) == precedence) {
        std::string operatorStr = tokens.consume();
        possible_whitespace(tokens);
        Term* rightExpr = infix_expression_nested(branch, tokens, precedence+1);

        std::string functionName = getInfixFunctionName(operatorStr);
        Term* function = find_named(&branch, functionName);
        assert(function != NULL);

        leftExpr = apply_function(&branch, function, ReferenceList(leftExpr, rightExpr));
    }

    return leftExpr;
}

Term* atom(Branch& branch, TokenStream& tokens)
{
    // function call?
    if (tokens.nextIs(IDENTIFIER) && tokens.nextIs(LPAREN, 1))
        return function_call(branch, tokens);

    // literal integer?
    if (tokens.nextIs(INTEGER))
        return literal_integer(branch, tokens);

    // literal string?
    if (tokens.nextIs(STRING))
        return literal_string(branch, tokens);

    // literal float?
    if (tokens.nextIs(FLOAT))
        return literal_float(branch, tokens);

/*
    // literal hex?
    if (tokens.nextIs(HEX_INTEGER))
        return literal_hex(tokens);

    // rebind operator?
    bool hasRebindOperator = false;

    if (tokens.nextIs(AMPERSAND)) {
        hasRebindOperator = true;
        tokens.consume(AMPERSAND);
    }

    // identifier?
    if (tokens.nextIs(IDENTIFIER)) {
        ast::Identifier* id = new ast::Identifier(tokens.consume(tokenizer::IDENTIFIER));
        id->hasRebindOperator = hasRebindOperator;
        return id;
    } else if (hasRebindOperator) {
        throw std::runtime_error("@ operator only allowed before an identifier");
    }
*/

    // parenthesized expression?
    if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        Term* result = infix_expression(branch, tokens);
        tokens.consume(RPAREN);
        return result;
    }

    throw std::runtime_error("unrecognized expression");

    return NULL; // unreachable
}

Term* function_call(Branch& branch, TokenStream& tokens)
{
    // todo
    return NULL;
}

Term* literal_integer(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(INTEGER);
    int value = strtol(text.c_str(), NULL, 0);
    Term* term = int_value(branch, value);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

Term* literal_float(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(FLOAT);
    float value = atof(text.c_str());
    Term* term = float_value(branch, value);
    // float mutability = ast.hasQuestionMark ? 1.0 : 0.0;
    // term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

Term* literal_string(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(STRING);
    Term* term = string_value(branch, text);
    term->syntaxHints.declarationStyle = TermSyntaxHints::LITERAL_VALUE;
    term->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    return term;
}

std::string possible_whitespace(TokenStream& tokens)
{
    if (tokens.nextIs(WHITESPACE))
        return tokens.consume(WHITESPACE);
    else
        return "";
}

std::string possible_newline(TokenStream& tokens)
{
    if (tokens.nextIs(NEWLINE))
        return tokens.consume(NEWLINE);
    else
        return "";
}

std::string possible_whitespace_or_newline(TokenStream& tokens)
{
    std::stringstream output;

    while (tokens.nextIs(NEWLINE) || tokens.nextIs(WHITESPACE))
        output << tokens.consume();

    return output.str();
}

} // namespace parser
} // namespace circa
