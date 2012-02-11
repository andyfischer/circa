// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"

#include "string_type.h"
#include "names.h"
#include "tagged_value.h"
#include "token.h"

namespace circa {

const char* get_token_text(int match)
{
    switch (match) {
        case TK_LPAREN: return "(";
        case TK_RPAREN: return ")";
        case TK_LBRACE: return "{";
        case TK_RBRACE: return "}";
        case TK_LBRACKET: return "[";
        case TK_RBRACKET: return "]";
        case TK_COMMA: return ",";
        case TK_AT_SIGN: return "@";
        case TK_IDENTIFIER: return "IDENTIFIER";
        case TK_SYMBOL: return "SYMBOL";
        case TK_INTEGER: return "INTEGER";
        case TK_HEX_INTEGER: return "HEX_INTEGER";
        case TK_FLOAT: return "FLOAT";
        case TK_STRING: return "STRING";
        case TK_COLOR: return "COLOR";
        case TK_COMMENT: return "COMMENT";
        case TK_DOT: return ".";
        case TK_STAR: return "*";
        case TK_QUESTION: return "?";
        case TK_SLASH: return "/";
        case TK_DOUBLE_SLASH: return "//";
        case TK_PLUS: return "+";
        case TK_MINUS: return "-";
        case TK_LTHAN: return "<";
        case TK_LTHANEQ: return "<=";
        case TK_GTHAN: return ">";
        case TK_GTHANEQ: return ">=";
        case TK_PERCENT: return "%";
        case TK_COLON: return ":";
        case TK_DOUBLE_COLON: return "::";
        case TK_DOUBLE_EQUALS: return "==";
        case TK_NOT_EQUALS: return "!=";
        case TK_EQUALS: return "=";
        case TK_PLUS_EQUALS: return "+=";
        case TK_MINUS_EQUALS: return "-=";
        case TK_STAR_EQUALS: return "*=";
        case TK_SLASH_EQUALS: return "/=";
        case TK_COLON_EQUALS: return ":=";
        case TK_RIGHT_ARROW: return "->";
        case TK_LEFT_ARROW: return "<-";
        case TK_AMPERSAND: return "&";
        case TK_DOUBLE_AMPERSAND: return "&&";
        case TK_DOUBLE_VERTICAL_BAR: return "||";
        case TK_SEMICOLON: return ";";
        case TK_TWO_DOTS: return "..";
        case TK_ELLIPSIS: return "...";
        case TK_TRIPLE_LTHAN: return "<<<";
        case TK_TRIPLE_GTHAN: return ">>>";
        case TK_POUND: return "#";
        case TK_WHITESPACE: return "WHITESPACE";
        case TK_NEWLINE: return "NEWLINE";
        case TK_BEGIN: return "begin";
        case TK_DO: return "do";
        case TK_END: return "end";
        case TK_IF: return "if";
        case TK_ELSE: return "else";
        case TK_ELIF: return "elif";
        case TK_FOR: return "for";
        case TK_STATE: return "state";
        case TK_DEF: return "def";
        case TK_TYPE: return "type";
        case TK_RETURN: return "return";
        case TK_IN: return "in";
        case TK_TRUE: return "true";
        case TK_FALSE: return "false";
        case TK_DO_ONCE: return "do once";
        case TK_NAMESPACE: return "namespace";
        case TK_INCLUDE: return "include";
        case TK_IMPORT: return "import";
        case TK_AND: return "and";
        case TK_OR: return "or";
        case TK_DISCARD: return "discard";
        case TK_NULL: return "null";
        case TK_BREAK: return "break";
        case TK_CONTINUE: return "continue";
        case TK_SWITCH: return "switch";
        case TK_CASE: return "case";
        case TK_UNRECOGNIZED: return "UNRECOGNIZED";
        default: return "NOT FOUND";
    }
}

std::string Token::toString() const
{
    std::stringstream out;
    out << get_token_text(match);
    return out.str();
}
int Token::length() const
{
    return colEnd - colStart;
}

struct TokenizeContext
{
    std::string const &input;
    int nextIndex;
    int linePosition;
    int charPosition;
    int precedingIndent;
    std::vector<Token> *results;

    TokenizeContext(std::string const &_input, std::vector<Token> *_results)
        : input(_input),
          nextIndex(0),
          linePosition(1),
          charPosition(0),
          precedingIndent(-1),
          results(_results)
    { }

    char next(int lookahead=0) const
    {
        unsigned int index = nextIndex + lookahead;
        if (index >= input.length())
            return 0;
        return input[index];
    }

    char advanceChar()
    {
        if (finished())
            return 0;

        char c = next();
        this->nextIndex++;

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

    bool withinRange(int lookahead) const {
        return nextIndex + lookahead < (int) input.length();
    }

    void consume(int match, int len) {

        Token instance;
        instance.match = match;
        instance.charIndex = this->nextIndex;

        // Record where this token starts
        instance.lineStart = this->linePosition;
        instance.colStart = this->charPosition;

        for (int i=0; i < len; i++)
            advanceChar();

        // Record where this token ends
        instance.lineEnd = this->linePosition;
        instance.colEnd = this->charPosition;

        // Update precedingIndent if this is the first whitespace on a line
        if (this->precedingIndent == -1) {
            if (instance.match == TK_WHITESPACE)
                this->precedingIndent = len;
            else
                this->precedingIndent = 0;
        }

        // NEWLINE token is a special case
        if (instance.match == TK_NEWLINE) {
            instance.lineEnd = instance.lineStart;
            instance.colEnd = instance.colStart + 1;
            this->precedingIndent = -1;
        }

        instance.precedingIndent = this->precedingIndent;

        ca_assert(instance.lineStart >= 0);
        ca_assert(instance.lineEnd >= 0);
        ca_assert(instance.colStart >= 0);
        ca_assert(instance.colEnd >= 0);
        ca_assert(instance.lineStart <= instance.lineEnd);
        ca_assert((instance.colEnd > instance.colStart) ||
                instance.lineStart < instance.lineEnd);

        results->push_back(instance);
    }
};

void top_level_consume_token(TokenizeContext &context);
void consume_identifier(TokenizeContext &context);
void consume_symbol(TokenizeContext &context);
void consume_whitespace(TokenizeContext &context);
void consume_comment(TokenizeContext& context);
bool match_number(TokenizeContext &context);
void consume_number(TokenizeContext &context);
void consume_hex_number(TokenizeContext &context);
void consume_string_literal(TokenizeContext &context);
void consume_triple_quoted_string_literal(TokenizeContext &context);
void consume_color_literal(TokenizeContext &context);

void tokenize(std::string const &input, TokenList* results)
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

bool is_identifier_first_letter(char c)
{
    return is_letter(c) || c == '_';
}

bool is_acceptable_inside_identifier(char c)
{
    return is_letter(c) || is_number(c) || c == '_' || c == ':';
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
    int str_len = (int) strlen(str);

    // Check if every letter matches
    for (int i=0; i < str_len; i++) {
        if (context.next(i) != str[i])
            return false;
    }

    // Check that this is really the end of the word
    if (is_acceptable_inside_identifier(context.next(str_len)))
        return false;

    // Don't match as a keyword if the next character is (. This might be
    // a bad idea.
    if (context.next(str_len) == '(')
        return false;

    // Keyword matches, now consume it
    context.consume(keyword, str_len);

    return true;
}

void top_level_consume_token(TokenizeContext &context)
{
    if (is_identifier_first_letter(context.next())) {

        if (try_to_consume_keyword(context, TK_DEF)) return;
        if (try_to_consume_keyword(context, TK_TYPE)) return;
        if (try_to_consume_keyword(context, TK_BEGIN)) return;
        if (try_to_consume_keyword(context, TK_END)) return;
        if (try_to_consume_keyword(context, TK_IF)) return;
        if (try_to_consume_keyword(context, TK_ELSE)) return;
        if (try_to_consume_keyword(context, TK_ELIF)) return;
        if (try_to_consume_keyword(context, TK_FOR)) return;
        if (try_to_consume_keyword(context, TK_STATE)) return;
        if (try_to_consume_keyword(context, TK_IN)) return;
        if (try_to_consume_keyword(context, TK_TRUE)) return;
        if (try_to_consume_keyword(context, TK_FALSE)) return;
        // check 'do once' before 'do'
        if (try_to_consume_keyword(context, TK_DO_ONCE)) return; 
        if (try_to_consume_keyword(context, TK_DO)) return;
        if (try_to_consume_keyword(context, TK_NAMESPACE)) return;
        if (try_to_consume_keyword(context, TK_INCLUDE)) return;
        if (try_to_consume_keyword(context, TK_IMPORT)) return;
        if (try_to_consume_keyword(context, TK_AND)) return;
        if (try_to_consume_keyword(context, TK_OR)) return;
        if (try_to_consume_keyword(context, TK_DISCARD)) return;
        if (try_to_consume_keyword(context, TK_NULL)) return;
        if (try_to_consume_keyword(context, TK_RETURN)) return;
        if (try_to_consume_keyword(context, TK_BREAK)) return;
        if (try_to_consume_keyword(context, TK_CONTINUE)) return;
        if (try_to_consume_keyword(context, TK_SWITCH)) return;
        if (try_to_consume_keyword(context, TK_CASE)) return;

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
            context.consume(TK_LPAREN, 1);
            return;
        case ')':
            context.consume(TK_RPAREN, 1);
            return;
        case '{':
            context.consume(TK_LBRACE, 1);
            return;
        case '}':
            context.consume(TK_RBRACE, 1);
            return;
        case '[':
            context.consume(TK_LBRACKET, 1);
            return;
        case ']':
            context.consume(TK_RBRACKET, 1);
            return;
        case ',':
            context.consume(TK_COMMA, 1);
            return;
        case '@':
            context.consume(TK_AT_SIGN, 1);
            return;
        case '=':
            if (context.next(1) == '=') {
                context.consume(TK_DOUBLE_EQUALS, 2);
                return;
            } 

            context.consume(TK_EQUALS, 1);
            return;
        case '"':
        case '\'':
            consume_string_literal(context);
            return;
        case '\n':
            context.consume(TK_NEWLINE, 1);
            return;
        case '.':
            if (context.next(1) == '.') {
                if (context.next(2) == '.') {
                    context.consume(TK_ELLIPSIS, 3); 
                } else {
                    context.consume(TK_TWO_DOTS, 2);
                }
            } else {
                context.consume(TK_DOT, 1);
            }
            return;
        case '?':
            context.consume(TK_QUESTION, 1);
            return;
        case '*':
            if (context.next(1) == '=') {
                context.consume(TK_STAR_EQUALS, 2);
                return;
            }

            context.consume(TK_STAR, 1);
            return;
        case '/':
            if (context.next(1) == '=') {
                context.consume(TK_SLASH_EQUALS, 2);
                return;
            }
            if (context.next(1) == '/') {
                context.consume(TK_DOUBLE_SLASH, 2);
                return;
            }
            context.consume(TK_SLASH, 1);
            return;
        case '!':
            if (context.next(1) == '=') {
                context.consume(TK_NOT_EQUALS, 2);
                return;
            }
            break;

        case ':':
            if (context.next(1) == '=') {
                context.consume(TK_COLON_EQUALS, 2);
                return;
            }
            else if (context.next(1) == ':') {
                context.consume(TK_DOUBLE_COLON, 2);
                return;
            } else if (is_identifier_first_letter(context.next(1))) {
                return consume_symbol(context);
            }

            context.consume(TK_COLON, 1);
            return;
        case '+':
            if (context.next(1) == '=') {
                context.consume(TK_PLUS_EQUALS, 2);
            } else {
                context.consume(TK_PLUS, 1);
            }
            return;
        case '-':
            if (context.next(1) == '>') {
                context.consume(TK_RIGHT_ARROW, 2);
                return;
            }

            if (context.next(1) == '-')
                return consume_comment(context);

            if (context.next(1) == '=') {
                context.consume(TK_MINUS_EQUALS, 2);
                return;
            }

            context.consume(TK_MINUS, 1);
            return;

        case '<':
            if (context.next(1) == '<' && context.next(2) == '<') {
                consume_triple_quoted_string_literal(context);
                return;
            }

            if (context.next(1) == '=') {
                context.consume(TK_LTHANEQ, 2);
                return;
            }
            if (context.next(1) == '-') {
                context.consume(TK_LEFT_ARROW, 2);
                return;
            }
            context.consume(TK_LTHAN, 1);
            return;

        case '>':
            if (context.next(1) == '=') {
                context.consume(TK_GTHANEQ, 2);
                return;
            }
            context.consume(TK_GTHAN, 1);
            return;

        case '%':
            context.consume(TK_PERCENT, 1);
            return;

        case '|':
            if (context.next(1) == '|') {
                context.consume(TK_DOUBLE_VERTICAL_BAR, 2);
                return;
            }
            break;

        case '&':
            if (context.next(1) == '&') {
                context.consume(TK_DOUBLE_AMPERSAND, 2);
                return;
            }

            context.consume(TK_AMPERSAND, 1);
            return;

        case ';':
            context.consume(TK_SEMICOLON, 1);
            return;

        case '#':
            if (context.next(1) == ' ')
                return consume_comment(context);

            consume_color_literal(context);
            return;
    }

    // Fall through, consume the next letter as UNRECOGNIZED
    context.consume(TK_UNRECOGNIZED, 1);
}

void consume_identifier(TokenizeContext &context)
{
    int lookahead = 0;
    while (is_acceptable_inside_identifier(context.next(lookahead)))
        lookahead++;

    context.consume(TK_IDENTIFIER, lookahead);
}

void consume_whitespace(TokenizeContext &context)
{
    int lookahead = 0;
    while (is_whitespace(context.next(lookahead)))
        lookahead++;

    context.consume(TK_WHITESPACE, lookahead);
}

void consume_comment(TokenizeContext& context)
{
    int lookahead = 0;
    while (context.withinRange(lookahead) && !is_newline(context.next(lookahead)))
        lookahead++;

    context.consume(TK_COMMENT, lookahead);
}

bool match_number(TokenizeContext &context)
{
    int lookahead = 0;

    if (context.next(lookahead) == '.')
        lookahead++;

    if (is_number(context.next(lookahead)))
        return true;

    return false;
}

void consume_number(TokenizeContext &context)
{
    int lookahead = 0;
    bool minus_sign = false;
    bool dot_encountered = false;

    if (context.next(lookahead) == '-') {
        lookahead++;
        minus_sign = true;
    }

    while (true) {
        if (is_number(context.next(lookahead))) {
            lookahead++;
        } else if (context.next(lookahead) == '.') {
            // If we've already encountered a dot, finish and don't consume
            // this one.
            if (dot_encountered)
                break;

            // Special case: if this dot is followed by another dot, then it should
            // be tokenized as TWO_DOTS, so don't consume it here.
            if (context.next(lookahead+1) == '.')
                break;

            // Another special case, if the dot is followed by an identifier, then
            // don't consume it here. It might be an object call.
            if (is_identifier_first_letter(context.next(lookahead + 1)))
                break;

            // Otherwise, consume the dot
            lookahead++;
            dot_encountered = true;
        }
        else {
            break;
        }
    }

    if (dot_encountered)
        context.consume(TK_FLOAT, lookahead);
    else
        context.consume(TK_INTEGER, lookahead);
}

void consume_hex_number(TokenizeContext &context)
{
    int lookahead = 0;

    // consume the 0x part
    lookahead += 2;

    while (is_hexadecimal_digit(context.next(lookahead)))
        lookahead++;

    context.consume(TK_HEX_INTEGER, lookahead);
}

void consume_string_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // Consume starting quote, this can be ' or "
    char quote_type = context.next();
    lookahead++;

    bool escapedNext = false;
    while (context.withinRange(lookahead)) {
        char c = context.next(lookahead);

        if (c == quote_type && !escapedNext)
            break;

        escapedNext = false;
        if (c == '\\')
            escapedNext = true;

        lookahead++;
    }

    // consume ending quote
    lookahead++;

    context.consume(TK_STRING, lookahead);
}

void consume_triple_quoted_string_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // Consume initial <<<
    lookahead += 3;

    while (context.withinRange(lookahead) &&
            !(context.next(lookahead) == '>'
                && context.next(lookahead + 1) == '>'
                && context.next(lookahead + 2) == '>'))
        lookahead++;

    // Consume closing >>>
    lookahead += 3;
    context.consume(TK_STRING, lookahead);
}

void consume_color_literal(TokenizeContext &context)
{
    int lookahead = 0;

    // consume #
    lookahead++;

    while (is_hexadecimal_digit(context.next(lookahead)))
        lookahead++;

    int hex_digits = lookahead - 1;

    // acceptable lengths are 3, 4, 6 or 8 characters (not including #)
    if (hex_digits == 3 || hex_digits == 4 || hex_digits == 6 || hex_digits == 8)
        context.consume(TK_COLOR, lookahead);
    else
        context.consume(TK_UNRECOGNIZED, lookahead);
}

void consume_symbol(TokenizeContext &context)
{
    int lookahead = 0;

    // consume the leading :
    lookahead++;

    while (is_acceptable_inside_identifier(context.next(lookahead)))
        lookahead++;

    context.consume(TK_SYMBOL, lookahead);
}

void TokenStream::reset(TValue* inputString)
{
    reset(as_string(inputString));
}

Token const&
TokenStream::next(int lookahead) const
{
    int i = this->_position + lookahead;

    if (i >= (int) tokens.size())
        throw std::runtime_error("index out of bounds");

    if (i < 0)
        throw std::runtime_error("index < 0");

    return tokens[i];
}

std::string TokenStream::nextStr(int lookahead) const
{
    return _sourceText.substr(next(lookahead).charIndex, next(lookahead).length());
}

int
TokenStream::findNextNonWhitespace(int lookahead) const
{
    int index = this->_position;

    while (true) {

        if (index >= (int) tokens.size())
            return -1;

        if (tokens[index].match == TK_WHITESPACE) {
            index++;
            continue;
        }

        if (lookahead == 0)
            return index;

        lookahead--;
        index++;
    }
}

int
TokenStream::nextNonWhitespace(int lookahead) const
{
    int index = findNextNonWhitespace(lookahead);

    if (index == -1)
        return TK_EOF;

    return tokens[index].match;
}

bool TokenStream::nextIs(int match, int lookahead) const
{
    if ((this->_position + lookahead) >= tokens.size())
        return false;
        
    return next(lookahead).match == match;
}

void
TokenStream::consume(int match)
{
    if (finished())
        throw std::runtime_error(std::string("Unexpected EOF while looking for ")
                + get_token_text(match));

    if ((match != -1) && next().match != match) {
        std::stringstream msg;
        msg << "Unexpected token (expected " << get_token_text(match)
            << ", found " << get_token_text(next().match)
            << " '" << nextStr() << "')";
        throw std::runtime_error(msg.str());
    }

    _position++;
}
std::string
TokenStream::consumeStr(int match)
{
    std::string out = nextStr();
    consume(match);
    return out;
}

void
TokenStream::consumeStr(TValue* output, int match)
{
    set_string(output, nextStr());
    consume(match);
}

Name
TokenStream::consumeName(int match)
{
    Name value = name_from_string(nextStr().c_str());
    consume(match);
    return value;
}

bool
TokenStream::nextNonWhitespaceIs(int match, int lookahead) const
{
    return nextNonWhitespace(lookahead) == match;
}

int
TokenStream::getPosition() const
{
    return _position;
}

void
TokenStream::resetPosition(int loc)
{
    ca_assert(loc >= 0);
    _position = loc;
}

std::string
TokenStream::toString() const
{
    std::stringstream out;

    out << "{index: " << _position << ", ";
    out << "tokens: [";

    bool first = true;

    for (unsigned int i=0; i < tokens.size(); i++) {
        if (!first) out << ", ";
        out << tokens[i].toString();
        first = false;
    }
    out << "]}";
    return out.str();
}

void print_remaining_tokens(std::ostream& out, TokenStream& tokens)
{
    for (int i=0; i < tokens.remaining(); i++) {
        if (i != 0) out << " ";
        out << get_token_text(tokens.next(i).match);
        out << "(" << tokens.nextStr(i) << ")";
    }
}

} // namespace circa
