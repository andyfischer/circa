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

Term* evaluate_statement(Branch& branch, std::string const& input)
{
    Term* result = compile_statement(branch, input);
    evaluate_term(result);
    return result;
}

void set_input_syntax(Term* term, int index, Term* input)
{
    TermSyntaxHints::InputSyntax& syntax = term->syntaxHints.getInputSyntax(index);
    if (input->name == "") {
        syntax.style = TermSyntaxHints::InputSyntax::BY_SOURCE;
        syntax.name = "";
    } else {
        syntax.style = TermSyntaxHints::InputSyntax::BY_NAME;
        syntax.name = input->name;
    }
}

Term* statement_list(Branch& branch, TokenStream& tokens)
{
    Term* term = NULL;

    while (!tokens.finished())
        term = statement(branch, tokens);

    return term;
}

Term* statement(Branch& branch, TokenStream& tokens)
{
    std::string precedingWhitespace = possible_whitespace(tokens);

    Term* result = NULL;

    // Comment line
    if (tokens.nextIs(DOUBLE_MINUS)) {
        result = comment_statement(branch, tokens);
        assert(result != NULL);
    }

    // Blank line
    else if (tokens.finished() || tokens.nextIs(NEWLINE)) {
        result = blank_line(branch, tokens);
        assert(result != NULL);
    }

    // Function decl
    else if (tokens.nextIs(FUNCTION)) {
        result = function_decl(branch, tokens);
    }

    // Type decl
    else if (tokens.nextIs(TYPE)) {
        result = type_decl(branch, tokens);
    }

    // If block
    else if (tokens.nextIs(IF)) {
        result = if_block(branch, tokens);
    }

    // Stateful value decl
    else if (tokens.nextIs(STATE)) {
        result = stateful_value_decl(branch, tokens);
    }

    // Return statement
    else if (tokens.nextIs(RETURN)) {
        result = return_statement(branch, tokens);
    }

    // Expression statement
    else {
        result = expression_statement(branch, tokens);
        assert(result != NULL);
    }

    if (precedingWhitespace != "" && result != NULL)
        result->syntaxHints.precedingWhitespace = 
            precedingWhitespace + result->syntaxHints.precedingWhitespace;

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

    initialize_as_subroutine(func);

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
        } else {
            name = get_placeholder_name_for_index(func.inputProperties.size());
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

    if (func.outputType == NULL)
        func.outputType = VOID_TYPE;

    possible_whitespace_or_newline(tokens);

    while (!tokens.nextIs(END)) {
        statement(func.subroutineBranch, tokens);
    }

    tokens.consume(END);

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

Term* if_block(Branch& branch, TokenStream& tokens)
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

        possible_whitespace(tokens);
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
    tokens.consume(STATE);
    possible_whitespace(tokens);
    std::string typeName = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);
    std::string name = tokens.consume(IDENTIFIER);
    possible_whitespace(tokens);

    Term* initialValue = NULL;
    if (tokens.nextIs(EQUALS)) {
        tokens.consume(EQUALS);
        possible_whitespace(tokens);
        initialValue = infix_expression(branch, tokens);
    }

    possible_newline(tokens);

    Term* type = find_named(&branch, typeName);
    assert(type != NULL);

    Term* result = create_value(&branch, type, name);
    result->setIsStateful(true);

    if (initialValue != NULL)
        copy_value(initialValue, result);

    return result;
}

Term* expression_statement(Branch& branch, TokenStream& tokens)
{
    bool isNameBinding = tokens.nextIs(tokenizer::IDENTIFIER)
        && (tokens.nextIs(tokenizer::EQUALS, 1)
                || (tokens.nextIs(tokenizer::WHITESPACE,1)
                    && (tokens.nextIs(tokenizer::EQUALS,2))));

    // check for name binding
    std::string name;
    if (isNameBinding) {
        name = tokens.consume(tokenizer::IDENTIFIER);
        possible_whitespace(tokens);
        tokens.consume(EQUALS);
        possible_whitespace(tokens);
    }

    Term* result = infix_expression(branch, tokens);
    possible_newline(tokens);

    std::string pendingRebind = pop_pending_rebind(branch);

    if (pendingRebind != "") {
        if (name != "") {
            throw std::runtime_error("term has both a name and a pending rebind: "
                    + name + "," + pendingRebind);
        }

        name = pendingRebind;
    }

    if (name != "") {
        branch.bindName(result, name);
    }

    return result;
}

Term* return_statement(Branch& branch, TokenStream& tokens)
{
    tokens.consume(RETURN);
    possible_whitespace(tokens);

    Term* result = infix_expression(branch, tokens);
    possible_newline(tokens);

    branch.bindName(result, "#return");
    
    return result;
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
    else if (infix == ":=")
        return "apply-feedback";
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

        Term* result = NULL;

        if (operatorStr == ".") {
            // dot concatenated call

            std::string functionName = tokens.consume(IDENTIFIER);

            // Try to find this function
            Term* function = NULL;
           
            // Check member functions first
            Type& leftExprType = as_type(leftExpr->type);

            bool memberFunctionCall = false;
            if (leftExprType.memberFunctions.contains(functionName)) {
                function = leftExprType.memberFunctions[functionName];
                memberFunctionCall = true;
            } else {
                function = find_named(&branch, functionName);
            }

            assert(function != NULL);

            ReferenceList inputs(leftExpr);

            // Look for inputs
            if (tokens.nextIs(LPAREN)) {
                tokens.consume(LPAREN);

                while (!tokens.nextIs(RPAREN)) {
                    possible_whitespace(tokens);
                    Term* input = infix_expression(branch, tokens);
                    inputs.append(input);
                    possible_whitespace(tokens);

                    if (!tokens.nextIs(RPAREN))
                        tokens.consume(COMMA);
                }
                tokens.consume(RPAREN);
            }

            result = apply_function(&branch, function, inputs);

            if (memberFunctionCall && leftExpr->name != "") {
                branch.bindName(result, leftExpr->name);
            }

            result->syntaxHints.declarationStyle = TermSyntaxHints::DOT_CONCATENATION;

            set_input_syntax(result, 0, leftExpr);

        } else if (operatorStr == "->") {
            std::string functionName = tokens.consume(IDENTIFIER);
            possible_whitespace(tokens);

            ReferenceList inputs(leftExpr);

            result = find_and_apply_function(branch, functionName, inputs);

            result->syntaxHints.declarationStyle = TermSyntaxHints::ARROW_CONCATENATION;

            set_input_syntax(result, 0, leftExpr);

        } else {
            Term* rightExpr = infix_expression_nested(branch, tokens, precedence+1);

            std::string functionName = getInfixFunctionName(operatorStr);

            result = find_and_apply_function(branch, functionName, ReferenceList(leftExpr, rightExpr));
            result->syntaxHints.declarationStyle = TermSyntaxHints::INFIX;
            result->syntaxHints.functionName = operatorStr;

            set_input_syntax(result, 0, leftExpr);
            set_input_syntax(result, 1, rightExpr);
        }

        leftExpr = result;
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

    // literal hex?
    if (tokens.nextIs(HEX_INTEGER))
        return literal_hex(branch, tokens);

    // identifier?
    if (tokens.nextIs(IDENTIFIER) || tokens.nextIs(AMPERSAND)) {
        return identifier(branch, tokens);
    }

    // parenthesized expression?
    if (tokens.nextIs(LPAREN)) {
        tokens.consume(LPAREN);
        Term* result = infix_expression(branch, tokens);
        tokens.consume(RPAREN);
        return result;
    }

    std::cout << tokens.next().text;

    throw std::runtime_error("unrecognized expression at " 
        + tokens.next().locationAsString());

    return NULL; // unreachable
}

Term* function_call(Branch& branch, TokenStream& tokens)
{
    std::string functionName = tokens.consume(IDENTIFIER);
    tokens.consume(LPAREN);

    ReferenceList inputs;

    TermSyntaxHints::InputSyntaxList inputSyntax;

    while (!tokens.nextIs(RPAREN)) {
        possible_whitespace(tokens);
        Term* term = infix_expression(branch, tokens);
        possible_whitespace(tokens);

        inputs.append(term);

        if (term->name == "")
            inputSyntax.push_back(TermSyntaxHints::InputSyntax::bySource());
        else
            inputSyntax.push_back(TermSyntaxHints::InputSyntax::byName(term->name));

        if (!tokens.nextIs(RPAREN))
            tokens.consume(COMMA);
    }

    tokens.consume(RPAREN);
    
    Term* result = find_and_apply_function(branch, functionName, inputs);

    result->syntaxHints.declarationStyle = TermSyntaxHints::FUNCTION_CALL;
    result->syntaxHints.functionName = functionName;
    result->syntaxHints.occursInsideAnExpression = is_inside_expression(branch);
    result->syntaxHints.inputSyntax = inputSyntax;

    return result;
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

Term* literal_hex(Branch& branch, TokenStream& tokens)
{
    std::string text = tokens.consume(HEX_INTEGER);
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

    float mutability = 0.0;

    if (tokens.nextIs(QUESTION)) {
        tokens.consume(QUESTION);
        mutability = 1.0;
    }

    term->addProperty("mutability", FLOAT_TYPE)->asFloat() = mutability;
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

Term* identifier(Branch& branch, TokenStream& tokens)
{
    bool rebind = false;
    if (tokens.nextIs(AMPERSAND)) {
        tokens.consume(AMPERSAND);
        rebind = true;
    }

    std::string id = tokens.consume(IDENTIFIER);

    if (rebind)
        push_pending_rebind(branch, id);

    return find_named(&branch, id);
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
