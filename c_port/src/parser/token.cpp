
#include "token.h"

namespace token {

const char * LPAREN = "LPAREN";
const char * RPAREN = "RPAREN";
const char * COMMA = "COMMA";
const char * IDENTIFIER = "IDENTIFIER";
const char * INTEGER = "INTEGER";
const char * FLOAT = "FLOAT";
const char * STRING = "STRING";
const char * WHITESPACE = "WHITESPACE";
const char * NEWLINE = "NEWLINE";
const char * UNRECOGNIZED = "UNRECOGNIZED";

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    std::vector<TokenInstance> &results;

    TokenizeContext(std::string const &_input, std::vector<TokenInstance> &_results)
        : input(_input), results(_results), nextIndex(0)
    {
    }

    char next() const {
        if (finished())
            return 0;
        return input[nextIndex];
    }

    char consume() {
        if (finished())
            return 0;

        char c = next();
        nextIndex++;
        return c;
    }

    bool finished() const {
        return nextIndex >= input.length();
    }

    void pushResult(const char * match, std::string text) {
        TokenInstance instance;
        instance.match = match;
        instance.text = text;
        results.push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_number(TokenizeContext &context);

void tokenize(std::string const &input, std::vector<TokenInstance> &results)
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

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_' || c == '-';
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_letter(context.next())) {
        consume_identifier(context);
        return;
    }

    if (is_whitespace(context.next())) {
        consume_whitespace(context);
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
        case ',':
            context.consume();
            context.pushResult(COMMA, ",");
            return;
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

void consume_number(TokenizeContext &context)
{
    std::stringstream text;

    bool dot_encountered = false;

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

void consume_string_literal(TokenizeContext &context)
{
    std::stringstream text;

    // consume quote mark
    context.consume();

    while (context.next() != '"') {
        text << context.consume();
    }

    context.consume();
}

void debug_print_token_list(std::ostream &stream, TokenList &tokens)
{

}

} // namespace token
