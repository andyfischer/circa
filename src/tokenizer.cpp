
#include "tokenizer.h"

namespace circa {
namespace tokenizer {

const char * LPAREN = "LPAREN";
const char * RPAREN = "RPAREN";
const char * COMMA = "COMMA";
const char * EQUALS = "EQUALS";
const char * IDENTIFIER = "IDENTIFIER";
const char * INTEGER = "INTEGER";
const char * FLOAT = "FLOAT";
const char * STRING = "STRING";
const char * QUOTED_IDENTIFIER = "QUOTED_IDENTIFIER";
const char * WHITESPACE = "WHITESPACE";
const char * NEWLINE = "NEWLINE";
const char * UNRECOGNIZED = "UNRECOGNIZED";

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    std::vector<TokenInstance> &results;

    TokenizeContext(std::string const &_input, std::vector<TokenInstance> &_results)
        : input(_input), results(_results), nextIndex(0), linePosition(1), charPosition(1)
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

        if (c == '\n') {
            this->linePosition++;
            this->charPosition = 1;
        } else
            this->charPosition++;

        return c;
    }

    bool finished() const {
        return nextIndex >= input.length();
    }

    void pushResult(const char * match, std::string text) {
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
void consume_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);
void consume_quoted_identifier(TokenizeContext &context);

void tokenize(std::string const &input, TokenList &results)
{
    TokenizeContext context(input, results);

    while (!context.finished()) {
        top_level_consume_token(context);
    }
}

TokenList tokenize(std::string const& input)
{
    TokenList results;
    tokenize(input, results);
    return results;
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
        case '=':
            context.consume();
            context.pushResult(EQUALS, "=");
            return;
        case '"':
            consume_string_literal(context);
            return;
        case '\'':
            consume_quoted_identifier(context);
            return;
        case '\n':
            context.consume();
            context.pushResult(NEWLINE, "\n");
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

    context.pushResult(STRING, text.str());
}

void consume_quoted_identifier(TokenizeContext &context)
{
    std::stringstream text;

    // consume quote mark
    context.consume();

    while (is_acceptable_inside_identifier(context.next())) {
        text << context.consume();
    }

    context.pushResult(STRING, text.str());
}

} // namespace token
} // namespace circa
