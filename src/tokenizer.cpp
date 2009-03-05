// Copyright 2008 Paul Hodge

#include "tokenizer.h"

namespace circa {
namespace tokenizer {

const char* getMatchText(int match)
{
    switch (match) {
        case LPAREN: return "(";
        case RPAREN: return ")";
        case LBRACE: return "{";
        case RBRACE: return "}";
        case LBRACKET: return "[";
        case RBRACKET: return "]";
        case COMMA: return ",";
        case AMPERSAND: return "@";
        case IDENTIFIER: return "IDENTIFIER";
        case INTEGER: return "INTEGER";
        case HEX_INTEGER: return "HEX_INTEGER";
        case FLOAT: return "FLOAT";
        case STRING: return "STRING";
        case COMMENT: return "COMMENT";
        case DOT: return ".";
        case STAR: return "*";
        case QUESTION: return "?";
        case SLASH: return "/";
        case PLUS: return "+";
        case MINUS: return "-";
        case LTHAN: return "<";
        case LTHANEQ: return "<=";
        case GTHAN: return ">";
        case GTHANEQ: return ">=";
        case DOUBLE_EQUALS: return "==";
        case NOT_EQUALS: return "!=";
        case EQUALS: return "=";
        case PLUS_EQUALS: return "+=";
        case MINUS_EQUALS: return "-=";
        case STAR_EQUALS: return "*=";
        case SLASH_EQUALS: return "/=";
        case COLON_EQUALS: return ":=";
        case RIGHT_ARROW: return "->";
        case DOUBLE_AMPERSAND: return "&&";
        case DOUBLE_VERTICAL_BAR: return "||";
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
        case END: return "end";
        case IF: return "if";
        case ELSE: return "else";
        case STATE: return "state";
        case FUNCTION: return "function";
        case TYPE: return "type";
        case RETURN: return "return";
        case UNRECOGNIZED: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

std::string TokenInstance::toString() const
{
    std::stringstream out;
    out << getMatchText(match) << " \"" << text << "\"";
    return out.str();
}

std::string TokenInstance::locationAsString() const
{
    std::stringstream out;
    out << "line " << line << ", char " << character;
    return out.str();
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    std::vector<TokenInstance> &results;

    TokenizeContext(std::string const &_input, std::vector<TokenInstance> &_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(1),
          results(_results)
    {
    }

    char next(int lookahead=0) const {
        unsigned int index = nextIndex + lookahead;
        if (index >= input.length())
            return 0;
        return input[index];
    }

    char consume() {
        if (finished())
            return 0;

        char c = next();
        nextIndex++;

        if (c == '\n') {
            this->linePosition++;
            this->charPosition = 1;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= (int) input.length();
    }

    void pushResult(int match, std::string text = "") {
        if (text == "" && match != STRING)
            text = getMatchText(match);

        TokenInstance instance;
        instance.match = match;
        instance.text = text;

        instance.line = this->linePosition;

        // I think this will give us the last character of the word. Will fix.
        instance.character = this->charPosition;

        results.push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_comment(TokenizeContext& context);
void consume_number(TokenizeContext &context);
void consume_hex_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);

void tokenize(std::string const &input, TokenList &results)
{
    TokenizeContext context(input, results);

    while (!context.finished()) {
        top_level_consume_token(context);
    }
}

bool is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c)
{
    return (c >= '0' && c <= '9');
}

bool is_hexadecimal_digit(char c)
{
    return is_number(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_' || c == '-' || c == ':';
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

bool is_newline(char c)
{
    return c == '\n';
}

bool try_to_consume_keyword(TokenizeContext& context, int keyword)
{
    const char* str = getMatchText(keyword);
    int str_len = strlen(str);

    // Check if every letter matches
    for (int i=0; i < str_len; i++) {
        if (context.next(i) != str[i])
            return false;
    }

    // Check that this is really the end of the word
    if (is_acceptable_inside_identifier(context.next(str_len)))
        return false;

    // Keyword matches, now consume it
    for (int i=0; i < str_len; i++) {
        context.consume();
    }

    context.pushResult(keyword);

    return true;
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_letter(context.next()) || context.next() == '_') {

        if (try_to_consume_keyword(context, FUNCTION)) return;
        if (try_to_consume_keyword(context, TYPE)) return;
        if (try_to_consume_keyword(context, END)) return;
        if (try_to_consume_keyword(context, IF)) return;
        if (try_to_consume_keyword(context, ELSE)) return;
        if (try_to_consume_keyword(context, STATE)) return;
        if (try_to_consume_keyword(context, RETURN)) return;

        consume_identifier(context);
        return;
    }

    if (is_whitespace(context.next())) {
        consume_whitespace(context);
        return;
    }

    if (context.next() == '0'
        && context.next(1) == 'x') {
        consume_hex_number(context);
        return;
    }

    if (is_number(context.next())) {
        consume_number(context);
        return;
    }

    // Check for specific characters
    switch(context.next()) {
        case '(':
            context.consume();
            context.pushResult(LPAREN, "(");
            return;
        case ')':
            context.consume();
            context.pushResult(RPAREN, ")");
            return;
        case '{':
            context.consume();
            context.pushResult(LBRACE);
            return;
        case '}':
            context.consume();
            context.pushResult(RBRACE);
            return;
        case '[':
            context.consume();
            context.pushResult(LBRACKET);
            return;
        case ']':
            context.consume();
            context.pushResult(RBRACKET);
            return;
        case ',':
            context.consume();
            context.pushResult(COMMA);
            return;
        case '@':
            context.consume();
            context.pushResult(AMPERSAND);
            return;
        case '=':
            context.consume();

            if (context.next() == '=') {
                context.consume();
                context.pushResult(DOUBLE_EQUALS);
                return;
            } 

            context.pushResult(EQUALS);
            return;
        case '"':
        case '\'':
            consume_string_literal(context);
            return;
        case '\n':
            context.consume();
            context.pushResult(NEWLINE, "\n");
            return;
        case '.':
            if (is_number(context.next(1))) {
                consume_number(context);
                return;
            }
            context.consume();
            context.pushResult(DOT);
            return;
        case '?':
            context.consume();
            context.pushResult(QUESTION);
            return;
        case '*':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(STAR_EQUALS);
                return;
            }

            context.pushResult(STAR);
            return;
        case '/':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(SLASH_EQUALS);
                return;
            }
            context.pushResult(SLASH);
            return;
        case ':':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(COLON_EQUALS);
                return;
            }
            break; // fall through
        case '+':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(PLUS_EQUALS);
            } else {
                context.pushResult(PLUS);
            }
            return;
        case '-':
            if (is_number(context.next(1)) || context.next(1) == '.') {
                consume_number(context);
                return;
            }
            
            if (context.next(1) == '>') {
                context.consume();
                context.consume();
                context.pushResult(RIGHT_ARROW);
                return;
            }

            if (context.next(1) == '-') {
                consume_comment(context);
                return;
            }

            if (context.next(1) == '=') {
                context.consume();
                context.consume();
                context.pushResult(MINUS_EQUALS);
                return;
            }

            context.consume();
            context.pushResult(MINUS);
            return;

        case '<':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(LTHANEQ);
                return;
            }
            context.pushResult(LTHAN);
            return;

        case '>':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.pushResult(GTHANEQ);
                return;
            }
            context.pushResult(GTHAN);
            return;

        case '|':
            context.consume();
            if (context.next() == '|') {
                context.consume();
                context.pushResult(DOUBLE_VERTICAL_BAR);
                return;
            }
            break;

        case '&':
            context.consume();
            if (context.next() == '&') {
                context.consume();
                context.pushResult(DOUBLE_AMPERSAND);
                return;
            }
            break;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    std::stringstream text;
    text << context.consume();
    context.pushResult(UNRECOGNIZED, text.str());
}

void consume_identifier(TokenizeContext &context)
{
    std::stringstream text;

    while (is_acceptable_inside_identifier(context.next())) {
        text << context.consume();
    }

    context.pushResult(IDENTIFIER, text.str());
}

void consume_whitespace(TokenizeContext &context)
{
    std::stringstream text;

    while (is_whitespace(context.next())) {
        text << context.consume();
    }

    context.pushResult(WHITESPACE, text.str());
}

void consume_comment(TokenizeContext& context)
{
    std::stringstream text;

    // consume the -- part
    text << context.consume();
    text << context.consume();

    while (!context.finished() && !is_newline(context.next()))
        text << context.consume();

    // throw away the newline
    if (!context.finished())
        context.consume();

    context.pushResult(COMMENT, text.str());
}

void consume_number(TokenizeContext &context)
{
    std::stringstream text;

    bool minus_sign = false;
    bool dot_encountered = false;

    if (context.next() == '-') {
        text << context.consume();
        minus_sign = true;
    }

    while (true) {
        if (is_number(context.next())) {
            text << context.consume();
        }
        else if (context.next() == '.') {
            // If we've already encountered a dot, finish and don't consume
            // this one.
            if (dot_encountered)
                break;

            text << context.consume();
            dot_encountered = true;
        }
        else {
            break;
        }
    }

    if (dot_encountered) {
        context.pushResult(FLOAT, text.str());
    } else {
        context.pushResult(INTEGER, text.str());
    }
}

void consume_hex_number(TokenizeContext &context)
{
    std::stringstream text;

    // consume the 0x part
    text << context.consume();
    text << context.consume();

    while (is_hexadecimal_digit(context.next())) {
        text << context.consume();
    }

    context.pushResult(HEX_INTEGER, text.str());
}

void consume_string_literal(TokenizeContext &context)
{
    std::stringstream text;

    // consume starting quote
    char quote_type = context.consume();
    text << quote_type;

    while (context.next() != quote_type && !context.finished()) {
        text << context.consume();
    }

    // consume ending quote
    text << context.consume();

    context.pushResult(STRING, text.str());
}

} // namespace token
} // namespace circa
