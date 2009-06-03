// Copyright 2008 Paul Hodge

#include "tokenizer.h"

namespace circa {
namespace tokenizer {

const char* get_token_text(int match)
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
        case FLOAT_TOKEN: return "FLOAT";
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
        case PERCENT: return "%";
        case COLON: return ":";
        case DOUBLE_EQUALS: return "==";
        case NOT_EQUALS: return "!=";
        case EQUALS: return "=";
        case PLUS_EQUALS: return "+=";
        case MINUS_EQUALS: return "-=";
        case STAR_EQUALS: return "*=";
        case SLASH_EQUALS: return "/=";
        case COLON_EQUALS: return ":=";
        case RIGHT_ARROW: return "->";
        case LEFT_ARROW: return "<-";
        case DOUBLE_AMPERSAND: return "&&";
        case DOUBLE_VERTICAL_BAR: return "||";
        case SEMICOLON: return ";";
        case ELLIPSIS: return "...";
        case WHITESPACE: return "WHITESPACE";
        case NEWLINE: return "NEWLINE";
        case END: return "end";
        case IF: return "if";
        case ELSE: return "else";
        case FOR: return "for";
        case STATE: return "state";
        case DEF: return "def";
        case TYPE: return "type";
        case RETURN: return "return";
        case IN_TOKEN: return "in";
        case TRUE_TOKEN: return "true";
        case FALSE_TOKEN: return "false";
        case UNRECOGNIZED: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

std::string Token::toString() const
{
    std::stringstream out;
    out << get_token_text(match) << " \"" << text << "\"";
    return out.str();
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    std::vector<Token> &results;

    TokenizeContext(std::string const &_input, std::vector<Token> &_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(0),
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
            this->charPosition = 0;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= (int) input.length();
    }

    void push(int match, std::string text = "") {
        if (text == "" && match != STRING)
            text = get_token_text(match);

        Token instance;
        instance.match = match;
        instance.text = text;

        // Record where this token started
        if (results.size() == 0) {
            instance.lineStart = 1;
            instance.colStart = 0;
        } else {
            Token& prevToken = results[results.size()-1];
            if (prevToken.match == NEWLINE) {
                instance.lineStart = prevToken.lineEnd + 1;
                instance.colStart = 0;
            } else {
                instance.lineStart = prevToken.lineEnd;
                instance.colStart = prevToken.colEnd;
            }
        }

        // Record where this token ended
        if (instance.match == NEWLINE) {
            instance.lineEnd = this->linePosition - 1;
            instance.colEnd = instance.colStart + 1;
        } else {
            instance.lineEnd = this->linePosition;
            instance.colEnd = this->charPosition;
        }

        assert(instance.lineStart >= 0);
        assert(instance.lineEnd >= 0);
        assert(instance.colStart >= 0);
        assert(instance.colEnd >= 0);
        assert(instance.lineStart <= instance.lineEnd);
        assert((instance.colEnd > instance.colStart) ||
                instance.lineStart < instance.lineEnd);

        results.push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_comment(TokenizeContext& context);
bool match_number(TokenizeContext &context);
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
    return is_letter(c) || is_number(c) || c == '_';
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
    const char* str = get_token_text(keyword);
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

    context.push(keyword);

    return true;
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_letter(context.next()) || context.next() == '_') {

        if (try_to_consume_keyword(context, DEF)) return;
        if (try_to_consume_keyword(context, TYPE)) return;
        if (try_to_consume_keyword(context, END)) return;
        if (try_to_consume_keyword(context, IF)) return;
        if (try_to_consume_keyword(context, ELSE)) return;
        if (try_to_consume_keyword(context, FOR)) return;
        if (try_to_consume_keyword(context, STATE)) return;
        if (try_to_consume_keyword(context, RETURN)) return;
        if (try_to_consume_keyword(context, IN_TOKEN)) return;
        if (try_to_consume_keyword(context, TRUE_TOKEN)) return;
        if (try_to_consume_keyword(context, FALSE_TOKEN)) return;

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

    if (match_number(context)) {
        consume_number(context);
        return;
    }

    // Check for specific characters
    switch(context.next()) {
        case '(':
            context.consume();
            context.push(LPAREN, "(");
            return;
        case ')':
            context.consume();
            context.push(RPAREN, ")");
            return;
        case '{':
            context.consume();
            context.push(LBRACE);
            return;
        case '}':
            context.consume();
            context.push(RBRACE);
            return;
        case '[':
            context.consume();
            context.push(LBRACKET);
            return;
        case ']':
            context.consume();
            context.push(RBRACKET);
            return;
        case ',':
            context.consume();
            context.push(COMMA);
            return;
        case '@':
            context.consume();
            context.push(AMPERSAND);
            return;
        case '=':
            context.consume();

            if (context.next() == '=') {
                context.consume();
                context.push(DOUBLE_EQUALS);
                return;
            } 

            context.push(EQUALS);
            return;
        case '"':
        case '\'':
            consume_string_literal(context);
            return;
        case '\n':
            context.consume();
            context.push(NEWLINE, "\n");
            return;
        case '.':
            context.consume();

            if ((context.next(0) == '.') && (context.next(1) == '.')) {
                context.consume();
                context.consume();
                context.push(ELLIPSIS);
                return;
            }

            context.push(DOT);
            return;
        case '?':
            context.consume();
            context.push(QUESTION);
            return;
        case '*':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(STAR_EQUALS);
                return;
            }

            context.push(STAR);
            return;
        case '/':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(SLASH_EQUALS);
                return;
            }
            context.push(SLASH);
            return;
        case ':':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(COLON_EQUALS);
                return;
            }

            context.push(COLON);
            return;
        case '+':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(PLUS_EQUALS);
            } else {
                context.push(PLUS);
            }
            return;
        case '-':
            if (context.next(1) == '>') {
                context.consume();
                context.consume();
                context.push(RIGHT_ARROW);
                return;
            }

            if (context.next(1) == '-') {
                consume_comment(context);
                return;
            }

            if (context.next(1) == '=') {
                context.consume();
                context.consume();
                context.push(MINUS_EQUALS);
                return;
            }

            context.consume();
            context.push(MINUS);
            return;

        case '<':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(LTHANEQ);
                return;
            }
            if (context.next() == '-') {
                context.consume();
                context.push(LEFT_ARROW);
                return;
            }
            context.push(LTHAN);
            return;

        case '>':
            context.consume();
            if (context.next() == '=') {
                context.consume();
                context.push(GTHANEQ);
                return;
            }
            context.push(GTHAN);
            return;

        case '%':
            context.consume();
            context.push(PERCENT);
            return;

        case '|':
            if (context.next(1) == '|') {
                context.consume();
                context.consume();
                context.push(DOUBLE_VERTICAL_BAR);
                return;
            }
            break;

        case '&':
            if (context.next(1) == '&') {
                context.consume();
                context.consume();
                context.push(DOUBLE_AMPERSAND);
                return;
            }
            break;

        case ';':
            if (context.next() == ';') {
                context.consume();
                context.push(SEMICOLON);
                return;
            }
            break;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    std::stringstream text;
    text << context.consume();
    context.push(UNRECOGNIZED, text.str());
}

void consume_identifier(TokenizeContext &context)
{
    std::stringstream text;

    while (is_acceptable_inside_identifier(context.next()))
        text << context.consume();

    context.push(IDENTIFIER, text.str());
}

void consume_whitespace(TokenizeContext &context)
{
    std::stringstream text;

    while (is_whitespace(context.next())) {
        text << context.consume();
    }

    context.push(WHITESPACE, text.str());
}

void consume_comment(TokenizeContext& context)
{
    std::stringstream text;

    // consume the -- part
    text << context.consume();
    text << context.consume();

    while (!context.finished() && !is_newline(context.next()))
        text << context.consume();

    // consume the newline
    //if (!context.finished())
    //    text << context.consume();

    context.push(COMMENT, text.str());
}

bool match_number(TokenizeContext &context)
{
    int lookahead = 0;

    if (context.next(lookahead) == '-')
        lookahead++;

    if (context.next(lookahead) == '.')
        lookahead++;

    if (is_number(context.next(lookahead)))
        return true;

    return false;
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
        context.push(FLOAT_TOKEN, text.str());
    } else {
        context.push(INTEGER, text.str());
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

    context.push(HEX_INTEGER, text.str());
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

    context.push(STRING, text.str());
}

} // namespace token
} // namespace circa
